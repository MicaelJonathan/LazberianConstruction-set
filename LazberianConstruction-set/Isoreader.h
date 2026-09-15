#pragma once
#include <cstdint>
#include <string>

// Localize files following ISO9660

struct IsoFileLocation {
    bool found = false;
    uint32_t lba = 0;        // Logical Block Address
    uint64_t offset = 0;     // Offset bytes inside ISO (lba * 2048)
    uint64_t sizeBytes = 0;  // Total filesize
};

IsoFileLocation FindFileInIso(const std::string& isoPath, const std::string& targetFileName);