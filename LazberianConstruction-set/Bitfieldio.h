#pragma once
#include <cstdint>
#include <string>

inline uint32_t MaxValueForBits(int xBits) {
    return (xBits >= 32) ? 0xFFFFFFFFu : ((1u << xBits) - 1u);
}

uint32_t ReadBitsFromBuffer(const uint8_t* data, int xBits, int bitOffset);

void WriteBitsToBuffer(uint8_t* data, int xBits, int bitOffset, uint32_t value);

uint32_t ReadBitsFromFile(const std::string& filePath, uint64_t fileOffset, int xBits, int bitOffset);

bool WriteBitsToFile(const std::string& filePath, uint64_t fileOffset, int xBits, int bitOffset, uint32_t value);

int16_t DecodeSigned5Bits(uint32_t raw);
uint32_t EncodeSigned5Bits(int16_t value);

int32_t DecodeSignedBits(uint32_t raw, int bitWidth);
uint32_t EncodeSignedBits(int32_t value, int bitWidth);