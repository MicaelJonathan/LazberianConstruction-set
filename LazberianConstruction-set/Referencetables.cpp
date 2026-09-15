#include "ReferenceTables.h"

#include <fstream>
#include <sstream>

namespace {
    std::string Trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }
        return s.substr(start, end - start + 1);
    }

    void LoadIdNameFile(const std::string& path, std::unordered_map<uint32_t, std::string>& outMap) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return; 
        }

        std::string line;
        while (std::getline(file, line)) {
            line = Trim(line);
            if (line.empty()) {
                continue;
            }

            std::stringstream ss(line);
            std::string name, idStr;
            if (!std::getline(ss, name, ',')) continue;
            if (!std::getline(ss, idStr, ',')) continue;
            idStr = Trim(idStr);

            try {
                uint32_t id = static_cast<uint32_t>(std::stoul(idStr, nullptr, 16));
                outMap[id] = name;
            }
            catch (...) {
                continue; 
            }
        }
    }

    void LoadIdNameFileIdFirst(const std::string& path, std::unordered_map<uint32_t, std::string>& outMap) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            line = Trim(line);
            if (line.empty()) {
                continue;
            }

            std::stringstream ss(line);
            std::string idStr, name;
            if (!std::getline(ss, idStr, ',')) continue;
            if (!std::getline(ss, name, ',')) continue;
            idStr = Trim(idStr);

            try {
                uint32_t id = static_cast<uint32_t>(std::stoul(idStr, nullptr, 16));
                outMap[id] = name;
            }
            catch (...) {
                continue;
            }
        }
    }

    void LoadItemsFile(const std::string& path, std::vector<ItemDefinition>& outItems) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return; 
        }

        std::string line;
        while (std::getline(file, line)) {
            line = Trim(line);
            if (line.empty()) {
                continue;
            }

            std::stringstream ss(line);
            std::string name, idStr;
            if (!std::getline(ss, name, ',')) continue;
            if (!std::getline(ss, idStr, ',')) continue;
            idStr = Trim(idStr);

            try {
                ItemDefinition item;
                item.id = static_cast<uint32_t>(std::stoul(idStr, nullptr, 16));
                item.name = name;
                outItems.push_back(item);
            }
            catch (...) {
                continue;
            }
        }
    }
}

ReferenceTables ReferenceTables::LoadFromAssets(const std::string& refsFolder) {
    ReferenceTables tables;
    LoadIdNameFileIdFirst(refsFolder + "/individualunits", tables.individualUnits);
    LoadIdNameFile(refsFolder + "/classes", tables.classes);
    LoadItemsFile(refsFolder + "/items", tables.items);
    LoadItemsFile(refsFolder + "/classes", tables.classesOrdered); 
    return tables;
}

std::string ReferenceTables::ResolveName(uint32_t textId, uint32_t classId) const {
    auto it = individualUnits.find(textId);
    if (it != individualUnits.end()) {
        return it->second;
    }

    auto it2 = classes.find(classId);
    if (it2 != classes.end()) {
        return it2->second;
    }

    return "**********";
}