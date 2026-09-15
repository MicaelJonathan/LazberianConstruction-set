#pragma once
#include <cstdint>
#include <string>
#include "GrowthFieldSpecs.h"

struct GrowthRecord {
    bool valid = false;

    uint16_t hp = 0;
    uint16_t str = 0;
    uint16_t def = 0;
    uint16_t spd = 0;
    uint16_t mag = 0;

    uint16_t proficiency[static_cast<size_t>(GrowthProficiencyCategory::Count)] = {};
};

GrowthRecord ReadGrowthRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize,
    int languageUsed, uint32_t characterId);