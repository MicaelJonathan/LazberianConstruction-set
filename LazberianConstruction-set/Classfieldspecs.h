#pragma once
#include <cstdint>
#include <cstddef>
#include "UnitFieldSpecs.h"


inline constexpr int CLASS_BASE_BIT_WIDTH = 5;
inline constexpr uint32_t CLASS_STR_BYTE_OFFSET = 0;
inline constexpr int CLASS_STR_BIT_OFFSET = 0;
inline constexpr uint32_t CLASS_SPD_BYTE_OFFSET = 0;
inline constexpr int CLASS_SPD_BIT_OFFSET = 5;
inline constexpr uint32_t CLASS_DEF_BYTE_OFFSET = 1;
inline constexpr int CLASS_DEF_BIT_OFFSET = 2;
inline constexpr uint32_t CLASS_MAG_BYTE_OFFSET = 2;
inline constexpr int CLASS_MAG_BIT_OFFSET = 0;
inline constexpr uint32_t CLASS_HP_BYTE_OFFSET = 2;
inline constexpr int CLASS_HP_BIT_OFFSET = 5;

inline constexpr uint32_t CLASS_MOVE_BYTE_OFFSET = 8;
inline constexpr int CLASS_MOVE_BIT_OFFSET = 0;
inline constexpr int CLASS_MOVE_BIT_WIDTH = 4;

inline constexpr uint32_t CLASS_EXP_BYTE_OFFSET = 4;
inline constexpr int CLASS_EXP_BIT_OFFSET = 0;
inline constexpr int CLASS_EXP_BIT_WIDTH = 7;

//   type == Flier         -> "flying"
//   if not                -> "mounted"
//   else                  -> "unmounted"
inline constexpr uint32_t CLASS_MOUNTED_FLAG_BYTE_OFFSET = 3;
inline constexpr int CLASS_MOUNTED_FLAG_BIT_OFFSET = 4;

// Type is saved as a power of 2
inline constexpr uint32_t CLASS_TYPE_BYTE_OFFSET = 4;
inline constexpr int CLASS_TYPE_BIT_OFFSET = 7;
inline constexpr int CLASS_TYPE_BIT_WIDTH = 8;

inline constexpr uint32_t CLASS_MOVEMENT_BYTE_OFFSET = 6;
inline constexpr int CLASS_MOVEMENT_BIT_OFFSET = 1;
inline constexpr int CLASS_MOVEMENT_BIT_WIDTH = 4;

inline constexpr int CLASS_GROWTH_BIT_WIDTH = 7;
inline constexpr uint32_t CLASS_GROWTH_HP_BYTE_OFFSET = 24;
inline constexpr int CLASS_GROWTH_HP_BIT_OFFSET = 0;
inline constexpr uint32_t CLASS_GROWTH_STR_BYTE_OFFSET = 24;
inline constexpr int CLASS_GROWTH_STR_BIT_OFFSET = 7;
inline constexpr uint32_t CLASS_GROWTH_SPD_BYTE_OFFSET = 25;
inline constexpr int CLASS_GROWTH_SPD_BIT_OFFSET = 6;
inline constexpr uint32_t CLASS_GROWTH_DEF_BYTE_OFFSET = 26;
inline constexpr int CLASS_GROWTH_DEF_BIT_OFFSET = 5;
inline constexpr uint32_t CLASS_GROWTH_MAG_BYTE_OFFSET = 28;
inline constexpr int CLASS_GROWTH_MAG_BIT_OFFSET = 0;


struct ClassCapFieldSpec {
    WeaponCategory category;
    uint32_t byteOffset; // 100 bytes
    int bitOffset;
};

inline constexpr int CLASS_CAP_BIT_WIDTH = 6;

inline constexpr ClassCapFieldSpec ClassCapFieldSpecs[] = {
    { WeaponCategory::Knife,      28, 7 },
    { WeaponCategory::Sword,      29, 5 },
    { WeaponCategory::SpearLance, 30, 3 },
    { WeaponCategory::Axe,        31, 1 },
    { WeaponCategory::Bow,        32, 0 },
    { WeaponCategory::Crossbow,   32, 6 },
    { WeaponCategory::Fire,       33, 4 },
    { WeaponCategory::Thunder,    34, 2 },
    { WeaponCategory::Wind,       35, 0 },
    { WeaponCategory::Holy,       36, 0 },
    { WeaponCategory::Dark,       36, 6 },
    { WeaponCategory::SShield,    37, 4 },
    { WeaponCategory::MShield,    38, 2 },
    { WeaponCategory::LShield,    39, 0 },
};

// 1 bit per index
inline constexpr uint32_t CLASS_SKILL_FLAGS_BASE_OFFSET = 12;

inline constexpr size_t CLASS_RECORD_SIZE = 100;