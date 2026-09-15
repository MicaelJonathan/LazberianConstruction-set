#include "ItemRecordWriter.h"
#include "ItemFieldSpecs.h"
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

    uint32_t GetFieldValue(const ItemStats& stats, ItemNumericField field) {
        switch (field) {
        case ItemNumericField::Might: return stats.might;
        case ItemNumericField::Hex: return stats.hexValue;
        case ItemNumericField::Accuracy: return stats.accuracy;
        case ItemNumericField::Weight: return stats.weight;
        case ItemNumericField::MaxRange: return stats.maxRange;
        case ItemNumericField::MinRange: return stats.minRange;
        case ItemNumericField::Crit: return stats.crit;
        case ItemNumericField::Uses: return stats.uses;
        case ItemNumericField::Level: return stats.level;
        case ItemNumericField::Price: return stats.price;
        case ItemNumericField::Defense: return stats.defense;
        case ItemNumericField::Speed: return stats.speed;
        case ItemNumericField::Avoid: return stats.avoid;
        case ItemNumericField::Hit: return stats.hit;
        case ItemNumericField::Magic: return stats.magic;
        case ItemNumericField::Strength: return stats.strength;
        case ItemNumericField::Rounds: return stats.rounds;
        case ItemNumericField::FireRes: return stats.fireRes;
        case ItemNumericField::ThunderRes: return stats.thunderRes;
        case ItemNumericField::WindRes: return stats.windRes;
        case ItemNumericField::DarkRes: return stats.darkRes;
        case ItemNumericField::HolyRes: return stats.holyRes;
        case ItemNumericField::CritAvoidPenalty: return stats.critAvoidPenalty;
        default: return 0;
        }
    }
}

bool WriteItemStats(const std::string& isoPath, uint64_t dataOffset, int languageUsed, uint32_t itemId, const ItemStats& stats) {
    uint64_t itemRelativeOffset = ComputeItemOffset(languageUsed, itemId);

    // Write on both, they are copies.
    uint64_t bases[2] = {
        dataOffset + itemRelativeOffset,
        dataOffset + (ItemOffsets[languageUsed].second + static_cast<uint64_t>(itemId - 1) * ITEM_STRIDE)
    };

    std::cout << "Salvando item ID 0x" << std::hex << itemId << std::dec
        << " (idioma=" << (languageUsed == 0 ? "JP" : "EN") << ")..." << std::endl;

    bool allOk = true;
    auto write = [&](uint64_t fileOffset, int xBits, int bitOffset, uint32_t value) {
        if (!WriteBitsToFile(isoPath, fileOffset, xBits, bitOffset, value)) {
            allOk = false;
        }
        };

    for (uint64_t fileBase : bases) {
        for (const auto& spec : ItemFieldSpecs) {
            uint32_t maxAllowed = (spec.maxValueOverride != 0) ? spec.maxValueOverride : MaxValueForBits(spec.bitWidth);
            uint32_t clamped = Clamp(GetFieldValue(stats, spec.field), spec.minValue, maxAllowed);
            write(fileBase + spec.byteOffset, spec.bitWidth, spec.bitOffset, clamped);
        }

        uint32_t durability = Clamp(stats.durabilityIndex, 0, ITEM_DURABILITY_MAX);
        write(fileBase + ITEM_DURABILITY_BYTE_OFFSET, ITEM_DURABILITY_BIT_WIDTH, ITEM_DURABILITY_BIT_OFFSET, durability);

        // Effect rate id (offset+27, com +100) value (offset+26, 7 bits).
        if (stats.effectRateId < 0) {
            write(fileBase + ITEM_EFFECT_RATE_VALUE_BYTE_OFFSET, ITEM_EFFECT_RATE_VALUE_BIT_WIDTH, 0, 0);
            write(fileBase + ITEM_EFFECT_RATE_ID_BYTE_OFFSET, ITEM_EFFECT_RATE_ID_BIT_WIDTH, 0, 0);
        }
        else {
            uint32_t rateValue = Clamp(stats.effectRateValue, 0, MaxValueForBits(ITEM_EFFECT_RATE_VALUE_BIT_WIDTH));
            uint32_t rateId = static_cast<uint32_t>(stats.effectRateId) + 100;
            write(fileBase + ITEM_EFFECT_RATE_VALUE_BYTE_OFFSET, ITEM_EFFECT_RATE_VALUE_BIT_WIDTH, 0, rateValue);
            write(fileBase + ITEM_EFFECT_RATE_ID_BYTE_OFFSET, ITEM_EFFECT_RATE_ID_BIT_WIDTH, 0, rateId);
        }

        for (size_t effId = 0; effId < ItemEffects.size(); ++effId) {
            bool isActive = std::find(stats.activeEffectIds.begin(), stats.activeEffectIds.end(), static_cast<int>(effId)) != stats.activeEffectIds.end();
            size_t byteOffset = ITEM_EFFECT_FLAGS_BASE_OFFSET + effId / 8;
            int bitOffset = static_cast<int>(effId % 8);
            write(fileBase + byteOffset, 1, bitOffset, isActive ? 1 : 0);
        }
    }

    std::cout << (allOk ? "Item ID 0x" : "[ERROR] Item ID 0x") << std::hex << itemId << std::dec
        << (allOk ? " saved." : " NOT completely saved - check permissions/path to ISO.") << std::endl;
    return allOk;
}