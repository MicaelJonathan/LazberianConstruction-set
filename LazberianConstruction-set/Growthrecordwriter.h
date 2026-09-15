#pragma once
#include <cstdint>
#include <string>
#include "GrowthRecordReader.h"

bool WriteGrowthRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize,
    int languageUsed, uint32_t characterId, const GrowthRecord& record);