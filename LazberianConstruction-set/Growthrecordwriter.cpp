#include "GrowthRecordWriter.h"
#include "GameOffsets.h"
#include "BitFieldIO.h"

#include <iostream>

namespace {
    uint32_t Clamp(uint32_t value, uint32_t minValue, uint32_t maxValue) {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }
}

bool WriteGrowthRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize,
    int languageUsed, uint32_t characterId, const GrowthRecord& record) {
    if (characterId == 0) {
        std::cout << "[warning] Growth not saved: invalid characterId." << std::endl;
        return false;
    }

    uint64_t growthOffsetJp = ComputeGrowthOffset(languageUsed, characterId);

    uint64_t growthOffsetMirror = GrowthOffsets[languageUsed].second + static_cast<uint64_t>(characterId - 1) * GROWTH_STRIDE;

    if (growthOffsetJp + GROWTH_RECORD_SIZE > dataSize) {
        std::cout << "[warning] Growth not saved: calculated offset is outside DATA3.DAT." << std::endl;
        return false;
    }

    std::cout << "Saving growth (characterId=0x" << std::hex << characterId << std::dec << ")..." << std::endl;

    bool allOk = true;
    auto write = [&](uint64_t fileOffset, int xBits, int bitOffset, uint32_t value) {
        if (!WriteBitsToFile(isoPath, fileOffset, xBits, bitOffset, value)) {
            allOk = false;
        }
        };

    uint64_t bases[2] = { dataOffset + growthOffsetJp, dataOffset + growthOffsetMirror };

    for (uint64_t base : bases) {
        write(base + GROWTH_HP_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_HP_BIT_OFFSET, Clamp(record.hp, 0, MaxValueForBits(GROWTH_STAT_BIT_WIDTH)));
        write(base + GROWTH_STR_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_STR_BIT_OFFSET, Clamp(record.str, 0, MaxValueForBits(GROWTH_STAT_BIT_WIDTH)));
        write(base + GROWTH_DEF_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_DEF_BIT_OFFSET, Clamp(record.def, 0, MaxValueForBits(GROWTH_STAT_BIT_WIDTH)));
        write(base + GROWTH_SPD_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_SPD_BIT_OFFSET, Clamp(record.spd, 0, MaxValueForBits(GROWTH_STAT_BIT_WIDTH)));
        write(base + GROWTH_MAG_BYTE_OFFSET, GROWTH_STAT_BIT_WIDTH, GROWTH_MAG_BIT_OFFSET, Clamp(record.mag, 0, MaxValueForBits(GROWTH_STAT_BIT_WIDTH)));

        for (const auto& spec : GrowthProficiencyFieldSpecs) {
            uint32_t value = record.proficiency[static_cast<size_t>(spec.category)];
            write(base + spec.byteOffset, GROWTH_PROFICIENCY_BIT_WIDTH, spec.bitOffset,
                Clamp(value, 0, MaxValueForBits(GROWTH_PROFICIENCY_BIT_WIDTH)));
        }
    }

    std::cout << (allOk ? "Growth saved." : "[ERROR] Growth was not saved completely.") << std::endl;
    return allOk;
}