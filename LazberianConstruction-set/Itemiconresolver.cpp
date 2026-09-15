#include "ItemIconResolver.h"

#include <filesystem>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;

namespace {
    // If item ID 0x001 -> "0x001.png".
    std::string BuildExpectedFilename(uint32_t itemId) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::nouppercase << std::setfill('0') << std::setw(3) << itemId << ".png";
        return oss.str();
    }
}

std::string ResolveItemIconPath(uint32_t itemId, const std::string& itemsFolder) {
    std::string expectedFilename = BuildExpectedFilename(itemId);
    std::string expectedPath = itemsFolder + "/" + expectedFilename;
    std::string defaultPath = itemsFolder + "/default.png";

    if (fs::exists(expectedPath)) {
        std::cout << "Item icon loaded: " << expectedPath << std::endl;
        return expectedPath;
    }

    std::cout << "Icon not found for item ID 0x"
        << std::hex << std::nouppercase << std::setfill('0') << std::setw(3) << itemId << std::dec
        << " (expected: " << expectedFilename << "). Using default.png." << std::endl;

    return defaultPath;
}