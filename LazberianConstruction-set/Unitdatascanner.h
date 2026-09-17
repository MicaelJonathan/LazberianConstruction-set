#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

inline constexpr uint8_t UNITSEEK_SIGNATURE[8] = { 0x55, 0x4E, 0x49, 0x54, 0x00, 0x00, 0x00, 0x00 }; // "UNIT" + 4 zeroes, to avoid catching unrelated UNIT strings.
inline constexpr uint8_t ENDC_SIGNATURE[4] = { 0x45, 0x4E, 0x44, 0x43 };                             // "ENDC"

// Save to temp/unitdataRefSheet: "unitdataXXX = 0xADDRESS".
struct UnitDataEntry {
    std::string name;
    uint64_t address = 0;
};

// SCAN DATA3, SEEK UNIT to ENDC and document it.
// For every unitdata selected the software will do the rest
int ScanUnitData(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize);

// Read unitdataRefSheet and return a vector of entries.
std::vector<UnitDataEntry> LoadUnitDataRefSheet(const std::string& path = "temp/unitdataRefSheet");

// Read unitDataNameRef ("unitdataXXX = Display name") and return it as a lookup map.
std::unordered_map<std::string, std::string> LoadUnitDataNameRef(const std::string& path = "assets/refs/unitdataNameRef");