#pragma once
#include <cstdint>
#include <string>
#include "ClassRecordReader.h"

bool WriteClassRecord(const std::string& isoPath, uint64_t dataOffset, int languageUsed, uint32_t classId, const ClassRecord& record);