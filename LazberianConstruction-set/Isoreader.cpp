#include "IsoReader.h"

#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace {
    constexpr uint32_t SECTOR_SIZE = 2048;
    constexpr uint32_t PVD_LBA = 16;

    uint32_t ReadUint32LE(const uint8_t* data) {
        return static_cast<uint32_t>(data[0])
            | (static_cast<uint32_t>(data[1]) << 8)
            | (static_cast<uint32_t>(data[2]) << 16)
            | (static_cast<uint32_t>(data[3]) << 24);
    }


    // Normalize filename the way ISO9660 store it.
    std::string NormalizeName(const std::string& name) {
        std::string result = name;
        size_t semicolon = result.find(';');
        if (semicolon != std::string::npos) {
            result = result.substr(0, semicolon);
        }
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return result;
    }

    struct DirEntry {
        uint32_t lba;
        uint64_t sizeBytes;
    };

    struct ParsedRecord {
        uint8_t recordLength = 0;
        uint32_t extentLba = 0;
        uint64_t dataLength = 0;
        bool isDirectory = false;
        std::string identifier;
    };

	// Interprets a directory record from the ISO9660 filesystem and extracts relevant information into a ParsedRecord structure.
    // Layout: https://wiki.osdev.org/ISO_9660#Directories
    ParsedRecord ParseDirectoryRecord(const uint8_t* record) {
        ParsedRecord result;
        result.recordLength = record[0];
        if (result.recordLength == 0) {
            return result;
        }

        result.extentLba = ReadUint32LE(record + 2);   // bytes 2-5 (LE)
        result.dataLength = ReadUint32LE(record + 10);  // bytes 10-13 (LE)

        uint8_t flags = record[25];
        result.isDirectory = (flags & 0x02) != 0;

        uint8_t idLength = record[32];
        result.identifier.assign(reinterpret_cast<const char*>(record + 33), idLength);

        return result;
    }
}

IsoFileLocation FindFileInIso(const std::string& isoPath, const std::string& targetFileName) {
    IsoFileLocation result;

    std::ifstream file(isoPath, std::ios::binary);
    if (!file.is_open()) {
        return result;
    }

    // --- Primary Volume Descriptor (sec 16) ---
    std::vector<uint8_t> pvdSector(SECTOR_SIZE);
    file.seekg(static_cast<std::streamoff>(PVD_LBA) * SECTOR_SIZE);
    file.read(reinterpret_cast<char*>(pvdSector.data()), SECTOR_SIZE);
    if (!file) {
        return result;
    }

    // Check if ISO9660.
    if (std::memcmp(pvdSector.data() + 1, "CD001", 5) != 0) {
        return result;
    }

    // Main dir on PVD, from 156 byte forward (34 bytes).
    ParsedRecord rootRecord = ParseDirectoryRecord(pvdSector.data() + 156);
    if (rootRecord.recordLength == 0) {
        return result;
    }

    const std::string normalizedTarget = NormalizeName(targetFileName);

    std::queue<DirEntry> pendingDirs;
    pendingDirs.push({ rootRecord.extentLba, rootRecord.dataLength });

    std::vector<uint8_t> dirBuffer;

    while (!pendingDirs.empty()) {
        DirEntry dir = pendingDirs.front();
        pendingDirs.pop();

        uint64_t sectorsToRead = (dir.sizeBytes + SECTOR_SIZE - 1) / SECTOR_SIZE;
        dirBuffer.assign(static_cast<size_t>(sectorsToRead) * SECTOR_SIZE, 0);

        file.seekg(static_cast<std::streamoff>(dir.lba) * SECTOR_SIZE);
        file.read(reinterpret_cast<char*>(dirBuffer.data()), dirBuffer.size());
        if (!file) {
            continue;
        }

        uint64_t pos = 0;
        while (pos < dir.sizeBytes) {
            uint8_t recordLength = dirBuffer[static_cast<size_t>(pos)];

            if (recordLength == 0) {
                // End of records if 0.
                pos = ((pos / SECTOR_SIZE) + 1) * SECTOR_SIZE;
                continue;
            }

            ParsedRecord record = ParseDirectoryRecord(dirBuffer.data() + static_cast<size_t>(pos));

            bool isSelfOrParent = (record.identifier.size() == 1 &&
                (record.identifier[0] == '\x00' || record.identifier[0] == '\x01'));

            if (!isSelfOrParent) {
                if (record.isDirectory) {
                    pendingDirs.push({ record.extentLba, record.dataLength });
                }
                else if (NormalizeName(record.identifier) == normalizedTarget) {
                    result.found = true;
                    result.lba = record.extentLba;
                    result.offset = static_cast<uint64_t>(record.extentLba) * SECTOR_SIZE;
                    result.sizeBytes = record.dataLength;
                    return result;
                }
            }

            pos += recordLength;
        }
    }

    return result;
}