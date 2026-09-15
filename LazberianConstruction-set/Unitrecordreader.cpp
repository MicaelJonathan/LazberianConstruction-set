#include "UnitRecordReader.h"
#include "BitFieldIO.h"
#include "GameTables.h"

#include <fstream>
#include <vector>

UnitRecord ReadUnitRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t characterOffset) {
    UnitRecord record;

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso.is_open()) {
        return record;
    }

    constexpr size_t WINDOW = 0xFC + 8;
    std::vector<uint8_t> buffer(WINDOW, 0);
    iso.seekg(static_cast<std::streamoff>(dataOffset + characterOffset));
    iso.read(reinterpret_cast<char*>(buffer.data()), WINDOW);
    if (!iso && !iso.eof()) {
        return record;
    }

    record.characterId = static_cast<uint16_t>(buffer[12] | (buffer[13] << 8));
    record.textId = static_cast<uint16_t>(buffer[14] | (buffer[15] << 8));
    record.portraitId = static_cast<uint16_t>(buffer[16] | (buffer[17] << 8));
    record.classId = static_cast<uint16_t>(buffer[19]);

    // Stats base
    record.level = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + 20, 6, 0));
    record.hp = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + 22, 7, 4));
    record.strength = DecodeSigned5Bits(ReadBitsFromBuffer(buffer.data() + 23, 5, 3));
    record.speed = DecodeSigned5Bits(ReadBitsFromBuffer(buffer.data() + 24, 5, 0));
    record.luck = DecodeSigned5Bits(ReadBitsFromBuffer(buffer.data() + 24, 5, 5));
    record.defense = DecodeSigned5Bits(ReadBitsFromBuffer(buffer.data() + 25, 5, 2));
    record.magic = DecodeSigned5Bits(ReadBitsFromBuffer(buffer.data() + 25, 5, 7));

    uint32_t offhandRaw = ReadBitsFromBuffer(buffer.data() + UNIT_OFFHAND_BYTE_OFFSET, UNIT_EQUIP_BIT_WIDTH, UNIT_OFFHAND_BIT_OFFSET);
    uint32_t mainhandRaw = ReadBitsFromBuffer(buffer.data() + UNIT_MAINHAND_BYTE_OFFSET, UNIT_EQUIP_BIT_WIDTH, UNIT_MAINHAND_BIT_OFFSET);
    record.offhandSlot = (offhandRaw == 0) ? -1 : static_cast<int>(offhandRaw - 1);
    record.mainhandSlot = (mainhandRaw == 0) ? -1 : static_cast<int>(mainhandRaw - 1);

    for (const auto& spec : ProficiencyFieldSpecs) {
        uint32_t value = ReadBitsFromBuffer(buffer.data() + spec.byteOffset, PROFICIENCY_BIT_WIDTH, spec.bitOffset);
        record.proficiency[static_cast<size_t>(spec.category)] = static_cast<uint16_t>(value);
    }

    for (size_t skillId = 0; skillId < Skills.size(); ++skillId) {
        size_t byteIndex = UNIT_SKILL_FLAGS_BASE_OFFSET + skillId / 8;
        int bitIndex = static_cast<int>(skillId % 8);
        if (byteIndex >= buffer.size()) {
            break;
        }
        uint32_t flag = ReadBitsFromBuffer(buffer.data() + byteIndex, 1, bitIndex);
        if (flag && Skills[skillId] != "--") {
            record.activeSkillIds.push_back(static_cast<int>(skillId));
        }
    }

    // Inventory: offset+0xBC.
    for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
        uint32_t slotOffset = static_cast<uint32_t>(UNIT_INVENTORY_BASE_OFFSET + slot * UNIT_INVENTORY_SLOT_SIZE);
        if (slotOffset + UNIT_INVENTORY_SLOT_SIZE > buffer.size()) {
            break;
        }
        InventorySlot& item = record.inventory[slot];
        item.itemId = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + slotOffset, 16, 0));
        item.durability = static_cast<uint8_t>(ReadBitsFromBuffer(buffer.data() + slotOffset + 2, 8, 4));
        item.dropped = ReadBitsFromBuffer(buffer.data() + slotOffset + 3, 1, 7) != 0;
        item.locked = ReadBitsFromBuffer(buffer.data() + slotOffset + 4, 1, 2) != 0;
    }

    return record;
}