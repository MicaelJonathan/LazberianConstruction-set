#pragma once
#include <cstdint>
#include <string>
#include <vector>


struct CharacterRecord {
    uint64_t offset = 0;
};

struct UnitGroup {
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::vector<CharacterRecord> characters;
};

UnitGroup ScanUnitGroup(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize, uint64_t groupStartOffset);