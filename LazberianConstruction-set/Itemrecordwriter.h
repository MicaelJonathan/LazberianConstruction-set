#pragma once
#include <cstdint>
#include <string>
#include "ItemRecordReader.h"

// DO NOT SAVE ANYTHING out of the mask!! No byte shifts in this house!!
bool WriteItemStats(const std::string& isoPath, uint64_t dataOffset, int languageUsed, uint32_t itemId, const ItemStats& stats);