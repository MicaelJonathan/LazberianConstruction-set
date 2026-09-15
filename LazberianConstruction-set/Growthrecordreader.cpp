#include "GrowthRecordReader.h"
#include "GameOffsets.h"
#include "BitFieldIO.h"

#include <fstream>
#include <vector>

GrowthRecord ReadGrowthRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize,
    int languageUsed, uint32_t characterId) {
    GrowthRecord record;

    if (characterId == 0) {
        return record; // no valid entry, no growth
    }

    uint64_t growthOffset = ComputeGrowthOffset(languageUsed, characterId);
    if (growthOffset + GROWTH_RECORD_SIZE > dataSize) {
        // Ignore values out of DATA3 range
        return record;
    }

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso.is_open()) {
        return record;
    }

    std::vector<uint8_t> buffer(GROWTH_RECORD_SIZE, 0);
    iso.seekg(static_cast<std::streamoff>(dataOffset + growthOffset));
    iso.read(reinterpret_cast<char*>(buffer.data()), GROWTH_RECORD_SIZE);
    if (!iso && !iso.eof()) {
        return record;
    }

    uint16_t hp = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + GROWTH_HP_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_HP_BIT_OFFSET));
    uint16_t str = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + GROWTH_STR_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_STR_BIT_OFFSET));
    uint16_t def = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + GROWTH_DEF_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_DEF_BIT_OFFSET));
    uint16_t spd = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + GROWTH_SPD_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_SPD_BIT_OFFSET));
    uint16_t mag = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + GROWTH_MAG_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_MAG_BIT_OFFSET));

    uint16_t proficiency[static_cast<size_t>(GrowthProficiencyCategory::Count)] = {};
    bool anyProficiencyNonZero = false;
    for (const auto& spec : GrowthProficiencyFieldSpecs) {
        uint16_t value = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + spec.byteOffset, GROWTH_PROFICIENCY_BIT_WIDTH, spec.bitOffset));
        proficiency[static_cast<size_t>(spec.category)] = value;
        if (value != 0) {
            anyProficiencyNonZero = true;
        }
    }

    bool anyStatNonZero = (hp || str || def || spd || mag);
    if (!anyStatNonZero && !anyProficiencyNonZero) {
        return record; 
    }

    record.valid = true;
    record.hp = hp;
    record.str = str;
    record.def = def;
    record.spd = spd;
    record.mag = mag;
    for (size_t i = 0; i < static_cast<size_t>(GrowthProficiencyCategory::Count); ++i) {
        record.proficiency[i] = proficiency[i];
    }

    return record;
}