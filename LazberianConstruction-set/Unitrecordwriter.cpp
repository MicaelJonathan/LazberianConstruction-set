#include "UnitRecordWriter.h"
#include "UnitFieldSpecs.h"
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

bool WriteUnitRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t characterOffset, const UnitRecord& record) {
    uint64_t base = dataOffset + characterOffset;

    bool allOk = true;
    auto write = [&](uint64_t fileOffset, int xBits, int bitOffset, uint32_t value) {
        if (!WriteBitsToFile(isoPath, fileOffset, xBits, bitOffset, value)) {
            allOk = false;
        }
        };

    std::cout << "Saving characters (characterId=0x" << std::hex << record.characterId << std::dec
        << ", offset 0x" << std::hex << characterOffset << std::dec << ")..." << std::endl;

    write(base + 12, 16, 0, record.characterId);
    write(base + 14, 16, 0, record.textId);
    write(base + 16, 16, 0, record.portraitId);
    // ClassID is one byte god dammit!!!!!!!!!!!!!
    write(base + 19, 8, 0, record.classId);

    write(base + 20, 6, 0, Clamp(record.level, 0, 63));
    write(base + 22, 7, 4, Clamp(record.hp, 0, 127));
    write(base + 23, 5, 3, EncodeSigned5Bits(record.strength));
    write(base + 24, 5, 0, EncodeSigned5Bits(record.speed));
    write(base + 24, 5, 5, EncodeSigned5Bits(record.luck));
    write(base + 25, 5, 2, EncodeSigned5Bits(record.defense));
    write(base + 25, 5, 7, EncodeSigned5Bits(record.magic));

    uint32_t offhandRaw = (record.offhandSlot < 0) ? 0 : static_cast<uint32_t>(record.offhandSlot + 1);
    uint32_t mainhandRaw = (record.mainhandSlot < 0) ? 0 : static_cast<uint32_t>(record.mainhandSlot + 1);
    write(base + UNIT_OFFHAND_BYTE_OFFSET, UNIT_EQUIP_BIT_WIDTH, UNIT_OFFHAND_BIT_OFFSET, Clamp(offhandRaw, 0, UNIT_INVENTORY_SLOT_COUNT));
    write(base + UNIT_MAINHAND_BYTE_OFFSET, UNIT_EQUIP_BIT_WIDTH, UNIT_MAINHAND_BIT_OFFSET, Clamp(mainhandRaw, 0, UNIT_INVENTORY_SLOT_COUNT));

    for (const auto& spec : ProficiencyFieldSpecs) {
        uint32_t value = record.proficiency[static_cast<size_t>(spec.category)];
        write(base + spec.byteOffset, PROFICIENCY_BIT_WIDTH, spec.bitOffset,
            Clamp(value, 0, MaxValueForBits(PROFICIENCY_BIT_WIDTH)));
    }

    for (size_t skillId = 0; skillId < Skills.size(); ++skillId) {
        bool isActive = std::find(record.activeSkillIds.begin(), record.activeSkillIds.end(),
            static_cast<int>(skillId)) != record.activeSkillIds.end();
        size_t byteIndex = UNIT_SKILL_FLAGS_BASE_OFFSET + skillId / 8;
        int bitIndex = static_cast<int>(skillId % 8);
        write(base + byteIndex, 1, bitIndex, isActive ? 1 : 0);
    }

    for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
        const InventorySlot& item = record.inventory[slot];
        uint64_t slotOffset = base + UNIT_INVENTORY_BASE_OFFSET + slot * UNIT_INVENTORY_SLOT_SIZE;
        write(slotOffset, 16, 0, item.itemId);
        write(slotOffset + 2, 8, 4, Clamp(item.durability, 0, 255));
        write(slotOffset + 3, 1, 7, item.dropped ? 1 : 0);
        write(slotOffset + 4, 1, 2, item.locked ? 1 : 0);
    }

    std::cout << (allOk ? "Character saved." : "[ERRO] Character NOT saved completely - check permissions/path of the ISO.") << std::endl;
    return allOk;
}