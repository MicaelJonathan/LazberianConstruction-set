#include "PortraitResolver.h"

#include <filesystem>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;

namespace {
    // 0x + 4Y's + .png
    std::string BuildExpectedFilename(uint32_t portraitId) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << portraitId << ".png";
        return oss.str();
    }
}

std::string ResolvePortraitPath(uint32_t portraitId, const std::string& facesFolder) {
    std::string expectedFilename = BuildExpectedFilename(portraitId);
    std::string expectedPath = facesFolder + "/" + expectedFilename;
    std::string defaultPath = facesFolder + "/default.png";

    if (fs::exists(expectedPath)) {
        std::cout << "Portrait loaded: " << expectedPath << std::endl;
        return expectedPath;
    }

    std::cout << "Portrait not found for ID 0x"
        << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << portraitId << std::dec
        << " (expected: " << expectedFilename << "). Using default.png." << std::endl;

    return defaultPath;
}