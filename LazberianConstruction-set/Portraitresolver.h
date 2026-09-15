#pragma once
#include <cstdint>
#include <string>

// 0xYYYY.png, fetch from faces folder
std::string ResolvePortraitPath(uint32_t portraitId, const std::string& facesFolder = "assets/faces");