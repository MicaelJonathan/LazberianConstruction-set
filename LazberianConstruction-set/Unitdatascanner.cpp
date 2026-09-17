#include "UnitDataScanner.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace {
    constexpr size_t UNIT_SIG_SIZE = 8;
    constexpr size_t ENDC_SIG_SIZE = 4;
    constexpr size_t STEP = 4;
    constexpr size_t CHUNK_SIZE = 16 * 1024 * 1024; // 16 MB at a time... again... RAM is expensive these days...

    enum class ScanState {
        SearchingUnit,
        SearchingEndc
    };

    bool MatchesSignature(const uint8_t* data, const uint8_t* signature, size_t length) {
        for (size_t i = 0; i < length; ++i) {
            if (data[i] != signature[i]) {
                return false;
            }
        }
        return true;
    }

    std::string FormatAddress(uint64_t address) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << address;
        return oss.str();
    }

    std::string FormatEntryName(int index) {
        std::ostringstream oss;
        oss << "unitdata" << std::setfill('0') << std::setw(3) << index;
        return oss.str();
    }
}

int ScanUnitData(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize) {
    std::filesystem::create_directory("temp");

    std::ofstream refSheet("temp/unitdataRefSheet", std::ios::out | std::ios::trunc);
    if (!refSheet.is_open()) {
        std::cout << "Error: not possible to create temp/unitdataRefSheet." << std::endl;
        return 0;
    }

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso.is_open()) {
        std::cout << "Error: not possible to reopen the ISO for scanning." << std::endl;
        return 0;
    }

    std::cout << "Starting scan for UNITSEEK in DATA3.DAT (" << dataSize << " bytes)..." << std::endl;

    // Buffer it, it's constantly missing the last bytes 
    std::vector<uint8_t> buffer(CHUNK_SIZE + UNIT_SIG_SIZE);
    size_t leftover = 0;
    uint64_t bufferBaseAddress = 0;
    uint64_t bytesConsumed = 0;

    ScanState state = ScanState::SearchingUnit;
    int unitCounter = 0;

    while (bytesConsumed < dataSize) {
        uint64_t remainingInFile = dataSize - bytesConsumed;
        size_t toRead = static_cast<size_t>(std::min<uint64_t>(CHUNK_SIZE, remainingInFile));

        iso.seekg(static_cast<std::streamoff>(dataOffset + bytesConsumed));
        iso.read(reinterpret_cast<char*>(buffer.data() + leftover), toRead);

        size_t validBytes = leftover + toRead;
        bytesConsumed += toRead;

        size_t pos = 0;

        if (validBytes >= UNIT_SIG_SIZE) {
            size_t scanLimit = validBytes - UNIT_SIG_SIZE;

            for (; pos <= scanLimit; pos += STEP) {
                uint64_t globalAddress = bufferBaseAddress + pos;

                if (state == ScanState::SearchingUnit) {
                    if (MatchesSignature(buffer.data() + pos, UNITSEEK_SIGNATURE, UNIT_SIG_SIZE)) {
                        ++unitCounter;
                        std::string name = FormatEntryName(unitCounter);
                        std::string addressStr = FormatAddress(globalAddress);

                        refSheet << name << " = " << addressStr << "\n";
                        std::cout << name << " found at: " << addressStr << std::endl;

                        state = ScanState::SearchingEndc;
                    }
                }
                else {
                    if (MatchesSignature(buffer.data() + pos, ENDC_SIGNATURE, ENDC_SIG_SIZE)) {
                        state = ScanState::SearchingUnit;
                    }
                }
            }
        }

        leftover = validBytes - pos;
        if (leftover > 0) {
            std::memmove(buffer.data(), buffer.data() + pos, leftover);
        }
        bufferBaseAddress += pos;
    }

    refSheet.close();

    std::cout << "Scan completed. " << unitCounter << " unit(s) found." << std::endl;
    std::cout << "References saved to: temp/unitdataRefSheet" << std::endl;

    return unitCounter;
}

std::vector<UnitDataEntry> LoadUnitDataRefSheet(const std::string& path) {
    std::vector<UnitDataEntry> entries;

    std::ifstream file(path);
    if (!file.is_open()) {
        return entries;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string name = line.substr(0, eqPos);
        std::string addressStr = line.substr(eqPos + 1);

        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end = s.find_last_not_of(" \t\r\n");
            s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
            };
        trim(name);
        trim(addressStr);

        try {
            UnitDataEntry entry;
            entry.name = name;
            entry.address = std::stoull(addressStr, nullptr, 16);
            entries.push_back(entry);
        }
        catch (...) {
            continue;
        }
    }

    return entries;
}

std::unordered_map<std::string, std::string> LoadUnitDataNameRef(const std::string& path) {
    std::unordered_map<std::string, std::string> nameRef;

    std::ifstream file(path);
    if (!file.is_open()) {
        return nameRef;
    }

    auto trim = [](std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
        };

    std::string line;
    while (std::getline(file, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string name = line.substr(0, eqPos);
        std::string displayName = line.substr(eqPos + 1);
        trim(name);
        trim(displayName);

        if (!name.empty()) {
            nameRef[name] = displayName;
        }
    }

    return nameRef;
}