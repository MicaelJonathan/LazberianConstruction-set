#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "UnitFieldSpecs.h"

struct InventorySlot {
    uint16_t itemId = 0; // 0 = empty slot
    uint8_t durability = 0;
    bool locked = false;
    bool dropped = false;
};

struct UnitRecord {
    // Identity (offsets +12 to +19)
    uint16_t characterId = 0; // +12/+13 - ID int, item and quantity
    uint16_t textId = 0;      // +14/+15 - ID for name
    uint16_t portraitId = 0;  // +16/+17 - ID for portrait
    uint16_t classId = 0;     // +18/+19 - ID for classes (IT'S ONE BYTE)

    uint16_t level = 0;
    uint16_t hp = 0;
    int16_t strength = 0;
    int16_t speed = 0;
    int16_t luck = 0;
    int16_t defense = 0;
    int16_t magic = 0;

    int mainhandSlot = -1;
    int offhandSlot = -1;

    // Show as raw
    uint16_t proficiency[static_cast<size_t>(WeaponCategory::Count)] = {};

    std::vector<int> activeSkillIds;

    // For the moment, bags are considered an item and nothing more.
    // TODO: Figure it out how to give bags with things inside.
    InventorySlot inventory[UNIT_INVENTORY_SLOT_COUNT];
};

UnitRecord ReadUnitRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t characterOffset);