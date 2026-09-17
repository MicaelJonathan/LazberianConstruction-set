#pragma once
#include <cstdint>
#include <string>
#include "ReferenceTables.h"

// .lep (Lazberian Export Project) raw bytes from disc saved straight to a file, easier to move around
bool ExportLepProject(const std::string& lepPath, const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize,
    int languageUsed, const ReferenceTables& referenceTables);

// write .lep back straight into disc
bool ImportLepProject(const std::string& lepPath, const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize);