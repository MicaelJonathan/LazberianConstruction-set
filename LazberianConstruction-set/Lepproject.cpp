#include "LepProject.h"
#include "UnitDataScanner.h"
#include "UnitGroupScanner.h"
#include "UnitRecordReader.h"
#include "GameOffsets.h"

#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

namespace {
    constexpr char LEP_MAGIC[4] = { 'L', 'E', 'P', '1' };
    constexpr uint8_t LEP_VERSION = 1;

    struct LepBlock {
        uint64_t relativeOffset = 0;
        std::vector<uint8_t> data;
    };

    bool ReadRawBlock(const std::string& isoPath, uint64_t absoluteOffset, uint64_t size, std::vector<uint8_t>& outData) {
        std::ifstream iso(isoPath, std::ios::binary);
        if (!iso.is_open()) {
            return false;
        }
        outData.assign(static_cast<size_t>(size), 0);
        iso.seekg(static_cast<std::streamoff>(absoluteOffset));
        iso.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(size));
        return static_cast<bool>(iso) || iso.eof();
    }

    void AppendMirroredBlocks(std::vector<LepBlock>& blocks, const std::string& isoPath, uint64_t dataOffset,
        const LanguageOffsetPair& offsets, uint64_t sizeBytes) {
        if (sizeBytes == 0) {
            return;
        }
        LepBlock first;
        first.relativeOffset = offsets.first;
        if (ReadRawBlock(isoPath, dataOffset + offsets.first, sizeBytes, first.data)) {
            blocks.push_back(std::move(first));
        }
        LepBlock second;
        second.relativeOffset = offsets.second;
        if (ReadRawBlock(isoPath, dataOffset + offsets.second, sizeBytes, second.data)) {
            blocks.push_back(std::move(second));
        }
    }

    void WriteU32(std::ofstream& out, uint32_t value) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void WriteU64(std::ofstream& out, uint64_t value) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    bool ReadU32(std::ifstream& in, uint32_t& value) {
        in.read(reinterpret_cast<char*>(&value), sizeof(value));
        return static_cast<bool>(in);
    }

    bool ReadU64(std::ifstream& in, uint64_t& value) {
        in.read(reinterpret_cast<char*>(&value), sizeof(value));
        return static_cast<bool>(in);
    }
}

bool ExportLepProject(const std::string& lepPath, const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize,
    int languageUsed, const ReferenceTables& referenceTables) {
    std::vector<LepBlock> blocks;
    uint32_t maxCharacterId = 0;

    std::vector<UnitDataEntry> unitEntries = LoadUnitDataRefSheet();
    std::cout << "Exporting " << unitEntries.size() << " unitdata group(s)..." << std::endl;

    for (const auto& entry : unitEntries) {
        UnitGroup group = ScanUnitGroup(isoPath, dataOffset, dataSize, entry.address);
        if (group.endOffset <= group.startOffset) {
            std::cout << "  [warning] Skipping " << entry.name << ": could not determine group end." << std::endl;
            continue;
        }

        uint64_t blockSize = (group.endOffset - group.startOffset) + 4; // +4 to include the ENDC signature
        if (group.startOffset + blockSize > dataSize) {
            std::cout << "  [warning] Skipping " << entry.name << ": group falls outside DATA3.DAT." << std::endl;
            continue;
        }

        LepBlock block;
        block.relativeOffset = group.startOffset;
        if (!ReadRawBlock(isoPath, dataOffset + group.startOffset, blockSize, block.data)) {
            std::cout << "  [warning] Skipping " << entry.name << ": could not read from ISO." << std::endl;
            continue;
        }
        blocks.push_back(std::move(block));

        for (const auto& character : group.characters) {
            UnitRecord record = ReadUnitRecord(isoPath, dataOffset, character.offset);
            maxCharacterId = std::max(maxCharacterId, static_cast<uint32_t>(record.characterId));
        }
    }

    uint32_t maxClassId = 0;
    for (const auto& cls : referenceTables.classesOrdered) {
        maxClassId = std::max(maxClassId, cls.id);
    }
    AppendMirroredBlocks(blocks, isoPath, dataOffset, ClassOffsets[languageUsed], static_cast<uint64_t>(maxClassId) * CLASS_STRIDE);

    uint32_t maxItemId = 0;
    for (const auto& item : referenceTables.items) {
        maxItemId = std::max(maxItemId, item.id);
    }
    AppendMirroredBlocks(blocks, isoPath, dataOffset, ItemOffsets[languageUsed], static_cast<uint64_t>(maxItemId) * ITEM_STRIDE);

    AppendMirroredBlocks(blocks, isoPath, dataOffset, GrowthOffsets[languageUsed], static_cast<uint64_t>(maxCharacterId) * GROWTH_STRIDE);

    std::cout << "Writing " << blocks.size() << " block(s) to " << lepPath << "..." << std::endl;

    std::ofstream out(lepPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        std::cout << "[ERROR] Could not create " << lepPath << "." << std::endl;
        return false;
    }

    out.write(LEP_MAGIC, sizeof(LEP_MAGIC));
    out.put(static_cast<char>(LEP_VERSION));
    out.put(static_cast<char>(languageUsed));
    WriteU32(out, static_cast<uint32_t>(blocks.size()));

    for (const auto& block : blocks) {
        WriteU64(out, block.relativeOffset);
        WriteU32(out, static_cast<uint32_t>(block.data.size()));
        out.write(reinterpret_cast<const char*>(block.data.data()), static_cast<std::streamsize>(block.data.size()));
    }

    bool ok = out.good();
    std::cout << (ok ? "Export completed." : "[ERROR] Export was not fully written.") << std::endl;
    return ok;
}

bool ImportLepProject(const std::string& lepPath, const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize) {
    std::ifstream in(lepPath, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "[ERROR] Could not open " << lepPath << "." << std::endl;
        return false;
    }

    char magic[4] = {};
    in.read(magic, sizeof(magic));
    if (std::memcmp(magic, LEP_MAGIC, sizeof(LEP_MAGIC)) != 0) {
        std::cout << "[ERROR] " << lepPath << " is not a valid .lep file." << std::endl;
        return false;
    }

    uint8_t version = static_cast<uint8_t>(in.get());
    uint8_t language = static_cast<uint8_t>(in.get());
    uint32_t blockCount = 0;
    if (!ReadU32(in, blockCount)) {
        std::cout << "[ERROR] " << lepPath << " is corrupted (missing block count)." << std::endl;
        return false;
    }

    std::cout << "Importing " << lepPath << " (version " << static_cast<int>(version)
        << ", language=" << (language == 0 ? "JP" : "EN") << ", " << blockCount << " block(s))..." << std::endl;

    std::fstream iso(isoPath, std::ios::in | std::ios::out | std::ios::binary);
    if (!iso.is_open()) {
        std::cout << "[ERROR] Could not open the ISO for writing." << std::endl;
        return false;
    }

    bool allOk = true;
    for (uint32_t i = 0; i < blockCount; ++i) {
        uint64_t relativeOffset = 0;
        uint32_t size = 0;
        if (!ReadU64(in, relativeOffset) || !ReadU32(in, size)) {
            std::cout << "[ERROR] " << lepPath << " is corrupted (block " << i << " header)." << std::endl;
            allOk = false;
            break;
        }

        std::vector<uint8_t> data(size);
        in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
        if (!in && !in.eof()) {
            std::cout << "[ERROR] " << lepPath << " is corrupted (block " << i << " data)." << std::endl;
            allOk = false;
            break;
        }

        if (relativeOffset + size > dataSize) {
            std::cout << "  [warning] Skipping block " << i << ": falls outside DATA3.DAT of the loaded ISO." << std::endl;
            allOk = false;
            continue;
        }

        iso.seekp(static_cast<std::streamoff>(dataOffset + relativeOffset));
        iso.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(size));
        if (!iso.good()) {
            std::cout << "  [ERROR] Failed writing block " << i << " to disk." << std::endl;
            allOk = false;
        }
    }

    std::cout << (allOk ? "Import completed." : "[ERROR] Import finished with errors - check the log above.") << std::endl;
    return allOk;
}