#pragma once
#include <cstdint>
#include <cstddef>

enum class GrowthProficiencyCategory {
    Knife, Sword, SpearLance, Axe, Bow, Crossbow, Fire, Thunder, Wind, Holy, Dark, Shield,
    Count
};

struct GrowthProficiencyFieldSpec {
    GrowthProficiencyCategory category;
    uint32_t byteOffset; // 32bytes
    int bitOffset;
};

inline constexpr int GROWTH_PROFICIENCY_BIT_WIDTH = 4;

inline constexpr GrowthProficiencyFieldSpec GrowthProficiencyFieldSpecs[] = {
    { GrowthProficiencyCategory::Knife,      8, 0 },
    { GrowthProficiencyCategory::Sword,      8, 4 },
    { GrowthProficiencyCategory::SpearLance, 9, 0 },
    { GrowthProficiencyCategory::Axe,        9, 4 },
    { GrowthProficiencyCategory::Bow,       10, 0 },
    { GrowthProficiencyCategory::Crossbow,  10, 4 },
    { GrowthProficiencyCategory::Fire,      11, 0 },
    { GrowthProficiencyCategory::Thunder,   11, 4 },
    { GrowthProficiencyCategory::Wind,      12, 0 },
    { GrowthProficiencyCategory::Holy,      12, 4 },
    { GrowthProficiencyCategory::Dark,      13, 0 },
    { GrowthProficiencyCategory::Shield,    13, 4 },
};

inline constexpr uint32_t GROWTH_HP_BYTE_OFFSET = 0;
inline constexpr int GROWTH_HP_BIT_OFFSET = 0;
inline constexpr uint32_t GROWTH_STR_BYTE_OFFSET = 0;
inline constexpr int GROWTH_STR_BIT_OFFSET = 7;
inline constexpr uint32_t GROWTH_DEF_BYTE_OFFSET = 1;
inline constexpr int GROWTH_DEF_BIT_OFFSET = 6;
inline constexpr uint32_t GROWTH_SPD_BYTE_OFFSET = 4;
inline constexpr int GROWTH_SPD_BIT_OFFSET = 2;
inline constexpr uint32_t GROWTH_MAG_BYTE_OFFSET = 5;
inline constexpr int GROWTH_MAG_BIT_OFFSET = 0;
inline constexpr int GROWTH_STAT_BIT_WIDTH = 7;

inline constexpr size_t GROWTH_RECORD_SIZE = 32;