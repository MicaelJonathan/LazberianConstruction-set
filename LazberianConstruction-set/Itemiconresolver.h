#pragma once
#include <cstdint>
#include <string>

// 0xYYY.png
std::string ResolveItemIconPath(uint32_t itemId, const std::string& itemsFolder = "assets/items");