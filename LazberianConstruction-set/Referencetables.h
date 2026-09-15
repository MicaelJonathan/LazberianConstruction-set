#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct ItemDefinition {
    uint32_t id;
    std::string name;
};

struct ReferenceTables {
    std::unordered_map<uint32_t, std::string> individualUnits;
    std::unordered_map<uint32_t, std::string> classes;

    std::vector<ItemDefinition> items;

    std::vector<ItemDefinition> classesOrdered;

    static ReferenceTables LoadFromAssets(const std::string& refsFolder = "assets/refs");

    std::string ResolveName(uint32_t textId, uint32_t classId) const;
};