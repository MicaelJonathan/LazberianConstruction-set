#include "ClassRecordWriter.h"
#include "GameOffsets.h"
#include "GameTables.h"
#include "BitFieldIO.h"

#include <iostream>
#include <algorithm>

namespace {
    uint32_t Clamp(uint32_t value, uint32_t minValue, uint32_t maxValue) {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }
}

bool WriteClassRecord(const std::string& isoPath, uint64_t dataOffset, int languageUsed, uint32_t classId, const ClassRecord& record) {
    if (classId == 0) {
        std::cout << "[warning] Class not saved: classId invalid." << std::endl;
        return false;
    }

    uint64_t offsetFirst = ComputeClassOffset(languageUsed, classId);
    uint64_t offsetSecond = ClassOffsets[languageUsed].second + static_cast<uint64_t>(classId - 1) * CLASS_STRIDE;

    std::cout << "Saving class (classId=0x" << std::hex << classId << std::dec << ")..." << std::endl;

    bool allOk = true;
    auto write = [&](uint64_t fileOffset, int xBits, int bitOffset, uint32_t value) {
        if (!WriteBitsToFile(isoPath, fileOffset, xBits, bitOffset, value)) {
            allOk = false;
        }
        };

    uint64_t bases[2] = { dataOffset + offsetFirst, dataOffset + offsetSecond };

    for (uint64_t base : bases) {
        write(base + CLASS_HP_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_HP_BIT_OFFSET, Clamp(record.hp, 0, MaxValueForBits(CLASS_BASE_BIT_WIDTH)));
        write(base + CLASS_STR_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_STR_BIT_OFFSET, Clamp(record.strength, 0, MaxValueForBits(CLASS_BASE_BIT_WIDTH)));
        write(base + CLASS_SPD_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_SPD_BIT_OFFSET, Clamp(record.speed, 0, MaxValueForBits(CLASS_BASE_BIT_WIDTH)));
        write(base + CLASS_DEF_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_DEF_BIT_OFFSET, Clamp(record.defense, 0, MaxValueForBits(CLASS_BASE_BIT_WIDTH)));
        write(base + CLASS_MAG_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_MAG_BIT_OFFSET, Clamp(record.magic, 0, MaxValueForBits(CLASS_BASE_BIT_WIDTH)));
        write(base + CLASS_MOVE_BYTE_OFFSET, CLASS_MOVE_BIT_WIDTH, CLASS_MOVE_BIT_OFFSET, Clamp(record.move, 0, MaxValueForBits(CLASS_MOVE_BIT_WIDTH)));
        write(base + CLASS_EXP_BYTE_OFFSET, CLASS_EXP_BIT_WIDTH, CLASS_EXP_BIT_OFFSET, Clamp(record.experience, 0, MaxValueForBits(CLASS_EXP_BIT_WIDTH)));

        uint32_t typeRaw = (record.typeIndex >= 0 && record.typeIndex < static_cast<int>(UnitTypes.size()))
            ? (1u << record.typeIndex) : 0;
        write(base + CLASS_TYPE_BYTE_OFFSET, CLASS_TYPE_BIT_WIDTH, CLASS_TYPE_BIT_OFFSET, typeRaw);

        bool isFlierNow = (record.typeIndex >= 0 && record.typeIndex < static_cast<int>(UnitTypes.size())
            && UnitTypes[record.typeIndex] == "Flier");
        bool wantsMounted = (record.mountIndex == 1) && !isFlierNow;
        write(base + CLASS_MOUNTED_FLAG_BYTE_OFFSET, 1, CLASS_MOUNTED_FLAG_BIT_OFFSET, wantsMounted ? 1u : 0u);

        write(base + CLASS_MOVEMENT_BYTE_OFFSET, CLASS_MOVEMENT_BIT_WIDTH, CLASS_MOVEMENT_BIT_OFFSET,
            Clamp(static_cast<uint32_t>(record.movementIndex), 0, static_cast<uint32_t>(MovementTypes.size() - 1)));

        write(base + CLASS_GROWTH_HP_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_HP_BIT_OFFSET, Clamp(record.growthHp, 0, MaxValueForBits(CLASS_GROWTH_BIT_WIDTH)));
        write(base + CLASS_GROWTH_STR_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_STR_BIT_OFFSET, Clamp(record.growthStrength, 0, MaxValueForBits(CLASS_GROWTH_BIT_WIDTH)));
        write(base + CLASS_GROWTH_SPD_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_SPD_BIT_OFFSET, Clamp(record.growthSpeed, 0, MaxValueForBits(CLASS_GROWTH_BIT_WIDTH)));
        write(base + CLASS_GROWTH_DEF_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_DEF_BIT_OFFSET, Clamp(record.growthDefense, 0, MaxValueForBits(CLASS_GROWTH_BIT_WIDTH)));
        write(base + CLASS_GROWTH_MAG_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_MAG_BIT_OFFSET, Clamp(record.growthMagic, 0, MaxValueForBits(CLASS_GROWTH_BIT_WIDTH)));

        for (const auto& spec : ClassCapFieldSpecs) {
            uint32_t value = record.caps[static_cast<size_t>(spec.category)];
            write(base + spec.byteOffset, CLASS_CAP_BIT_WIDTH, spec.bitOffset, Clamp(value, 0, MaxValueForBits(CLASS_CAP_BIT_WIDTH)));
        }

        for (size_t skillId = 0; skillId < Skills.size(); ++skillId) {
            bool isActive = std::find(record.activeSkillIds.begin(), record.activeSkillIds.end(),
                static_cast<int>(skillId)) != record.activeSkillIds.end();
            size_t byteIndex = CLASS_SKILL_FLAGS_BASE_OFFSET + skillId / 8;
            int bitIndex = static_cast<int>(skillId % 8);
            write(base + byteIndex, 1, bitIndex, isActive ? 1 : 0);
        }
    }

    std::cout << (allOk ? "Class saved." : "[ERROR] Class was NOT saved completely.") << std::endl;
    return allOk;
}