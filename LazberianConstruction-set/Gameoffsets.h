#pragma once
#include <cstdint>
#include <cstddef>

// Unitdata is searched on the fly.
// TODO move those values to a config file, so people don't need to recompile the software to support new languages.
struct LanguageOffsetPair {
    uint32_t first;
    uint32_t second;
};

inline constexpr LanguageOffsetPair GrowthOffsets[2] = {
    { 0x0CB4520, 0x08E907f4 }, // 0 = JP
    { 0x0CAAD20, 0x08EA7FF4 }, // 1 = ENG
};

inline constexpr LanguageOffsetPair ClassOffsets[2] = {
    { 0x0CAE5CC, 0x08E8A8A0 }, // 0 = JP
    { 0x0CA4DCC, 0x08EA20A0 }, // 1 = ENG
};

inline constexpr LanguageOffsetPair ItemOffsets[2] = {
    { 0x0CC0C04, 0x08E9CE08 }, // 0 = JP
    { 0x0CB7404, 0x08EB46D8 }, // 1 = ENG
};

inline constexpr size_t ITEM_STRIDE = 56;


inline uint64_t GetItemBaseOffset(int languageUsed) {
    return ItemOffsets[languageUsed].first;
}

inline uint64_t ComputeItemOffset(int languageUsed, uint32_t itemId) {
    return GetItemBaseOffset(languageUsed) + static_cast<uint64_t>(itemId - 1) * ITEM_STRIDE;
}

inline constexpr size_t GROWTH_STRIDE = 32;


inline uint64_t GetGrowthBaseOffset(int languageUsed) {
    return GrowthOffsets[languageUsed].first;
}


inline uint64_t ComputeGrowthOffset(int languageUsed, uint32_t characterId) {
    return GetGrowthBaseOffset(languageUsed) + static_cast<uint64_t>(characterId - 1) * GROWTH_STRIDE;
}

inline constexpr size_t CLASS_STRIDE = 100;


inline uint64_t GetClassBaseOffset(int languageUsed) {
    return ClassOffsets[languageUsed].first;
}

inline uint64_t ComputeClassOffset(int languageUsed, uint32_t classId) {
    return GetClassBaseOffset(languageUsed) + static_cast<uint64_t>(classId - 1) * CLASS_STRIDE;
}