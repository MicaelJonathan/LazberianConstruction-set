#pragma once
#include <cstdint>
#include <cstddef>

enum class WeaponCategory {
    Knife, Sword, SpearLance, Axe, Bow, Crossbow, Fire, Thunder, Wind, Holy, Dark, SShield, MShield, LShield,
    Count
};

struct ProficiencyFieldSpec {
    WeaponCategory category;
    uint32_t byteOffset;
    int bitOffset;
};

inline constexpr int PROFICIENCY_BIT_WIDTH = 10;

inline constexpr ProficiencyFieldSpec ProficiencyFieldSpecs[] = {
    { WeaponCategory::Knife,      36, 0 },
    { WeaponCategory::Sword,      37, 2 },
    { WeaponCategory::SpearLance, 38, 4 },
    { WeaponCategory::Axe,        40, 0 },
    { WeaponCategory::Bow,        41, 2 },
    { WeaponCategory::Crossbow,   42, 4 },
    { WeaponCategory::Fire,       44, 0 },
    { WeaponCategory::Thunder,    45, 2 },
    { WeaponCategory::Wind,       46, 4 },
    { WeaponCategory::Holy,       48, 0 },
    { WeaponCategory::Dark,       49, 2 },
    { WeaponCategory::SShield,    50, 4 },
    { WeaponCategory::MShield,    52, 0 },
    { WeaponCategory::LShield,    53, 2 },
};

inline constexpr uint32_t UNIT_OFFHAND_BYTE_OFFSET = 26;
inline constexpr int UNIT_OFFHAND_BIT_OFFSET = 4;
inline constexpr uint32_t UNIT_MAINHAND_BYTE_OFFSET = 27;
inline constexpr int UNIT_MAINHAND_BIT_OFFSET = 0;
inline constexpr int UNIT_EQUIP_BIT_WIDTH = 4;

inline constexpr uint32_t UNIT_SKILL_FLAGS_BASE_OFFSET = 56;

inline constexpr uint32_t UNIT_INVENTORY_BASE_OFFSET = 0xBC;
inline constexpr uint32_t UNIT_INVENTORY_SLOT_SIZE = 8;
inline constexpr size_t UNIT_INVENTORY_SLOT_COUNT = 8;