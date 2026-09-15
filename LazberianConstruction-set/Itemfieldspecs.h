#pragma once
#include <cstdint>

enum class ItemNumericField {
    Might, Hex, Accuracy, Weight, MaxRange, MinRange, Crit, Uses, Level, Price,
    Defense, Speed, Avoid, Hit, Magic, Strength, Rounds,
    FireRes, ThunderRes, WindRes, DarkRes, HolyRes, CritAvoidPenalty,
    Count
};

struct ItemFieldSpec {
    ItemNumericField field;
    uint32_t byteOffset; // 56 bytes per item
    int bitWidth;
    int bitOffset;
    uint32_t minValue;
    uint32_t maxValueOverride; 
};

inline constexpr ItemFieldSpec ItemFieldSpecs[] = {
    { ItemNumericField::Might,           0,  6, 5,   0,  0 },
    { ItemNumericField::Hex,             1,  4, 3,   0,  0 },
    { ItemNumericField::Accuracy,        1,  7, 7,   0,  0 },
    { ItemNumericField::Weight,          2,  5, 6,   0,  0 },
    { ItemNumericField::MaxRange,        3,  5, 3,   0,  0 },
    { ItemNumericField::MinRange,        4,  4, 0,   0,  0 },
    { ItemNumericField::Crit,            5,  7, 0,   0,  0 },
    { ItemNumericField::Uses,            5,  7, 7,   1,  0 },
    { ItemNumericField::Level,           6,  6, 6,   1, 50 },
    { ItemNumericField::Price,           8, 16, 0,   0,  0 },
    { ItemNumericField::Defense,        12,  6, 0,   0,  0 },
    { ItemNumericField::Speed,          13,  5, 3,   0,  0 },
    { ItemNumericField::Avoid,          14,  8, 0,   0,  0 },
    { ItemNumericField::Hit,            15,  8, 0,   0,  0 },
    { ItemNumericField::Magic,          16,  5, 0,   0,  0 },
    { ItemNumericField::Strength,       16,  5, 5,   0,  0 },
    { ItemNumericField::Rounds,         17,  4, 2,   0,  0 },
    { ItemNumericField::FireRes,        17,  6, 6,   0,  0 },
    { ItemNumericField::ThunderRes,     18,  6, 4,   0,  0 },
    { ItemNumericField::WindRes,        19,  6, 2,   0,  0 },
    { ItemNumericField::DarkRes,        20,  6, 0,   0,  0 },
    { ItemNumericField::HolyRes,        20,  6, 6,   0,  0 },
    { ItemNumericField::CritAvoidPenalty, 21, 8, 7,   0,  0 },
};

constexpr int ITEM_DURABILITY_BYTE_OFFSET = 21;
constexpr int ITEM_DURABILITY_BIT_WIDTH = 3;
constexpr int ITEM_DURABILITY_BIT_OFFSET = 4;
constexpr uint32_t ITEM_DURABILITY_MAX = 6; // It's 7, 0,1,2,3,4,5,6 = 7 values

constexpr int ITEM_EFFECT_RATE_VALUE_BYTE_OFFSET = 26;
constexpr int ITEM_EFFECT_RATE_VALUE_BIT_WIDTH = 7;
constexpr int ITEM_EFFECT_RATE_ID_BYTE_OFFSET = 27;
constexpr int ITEM_EFFECT_RATE_ID_BIT_WIDTH = 8;

constexpr int ITEM_EFFECT_FLAGS_BASE_OFFSET = 28;