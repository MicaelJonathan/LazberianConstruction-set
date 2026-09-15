#pragma once
#include <cstdint>
#include <string>
#include "UnitRecordReader.h"

bool WriteUnitRecord(const std::string& isoPath, uint64_t dataOffset, uint64_t characterOffset, const UnitRecord& record);