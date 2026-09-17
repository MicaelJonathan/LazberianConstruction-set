#include "BitFieldIO.h"

#include <fstream>

uint32_t ReadBitsFromBuffer(const uint8_t* data, int xBits, int bitOffset) {
    uint32_t mask = (xBits >= 8) ? 0xFFu : ((1u << xBits) - 1u);
    uint32_t value = ((mask << bitOffset) & 0xFFu & data[0]) >> bitOffset;
    if (bitOffset + xBits > 8) {
        uint32_t highMask = (1u << (bitOffset + xBits - 8)) - 1u;
        value += (data[1] & highMask) << (8 - bitOffset);
    }
    return value;
}

void WriteBitsToBuffer(uint8_t* data, int xBits, int bitOffset, uint32_t value) {
    // Mask the bits, avoid byte shift and corrupting other data
    value &= MaxValueForBits(xBits);

    uint32_t lowMask = (MaxValueForBits(xBits) << bitOffset) & 0xFFu;
    data[0] = static_cast<uint8_t>((data[0] & (0xFFu - lowMask)) + ((value << bitOffset) & 0xFFu));

    if (bitOffset + xBits > 8) {
        uint32_t highMaskOriginal = MaxValueForBits(xBits) >> (8 - bitOffset);
        data[1] = static_cast<uint8_t>((data[1] & (0xFFu - highMaskOriginal)) + (value >> (8 - bitOffset)));
    }
}

uint32_t ReadBitsFromFile(const std::string& filePath, uint64_t fileOffset, int xBits, int bitOffset) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return 0;
    }

    uint8_t buffer[2] = { 0, 0 };
    file.seekg(static_cast<std::streamoff>(fileOffset));
    file.read(reinterpret_cast<char*>(buffer), 2);

    return ReadBitsFromBuffer(buffer, xBits, bitOffset);
}

bool WriteBitsToFile(const std::string& filePath, uint64_t fileOffset, int xBits, int bitOffset, uint32_t value) {
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    uint8_t buffer[2] = { 0, 0 };
    file.seekg(static_cast<std::streamoff>(fileOffset));
    file.read(reinterpret_cast<char*>(buffer), 2);

    WriteBitsToBuffer(buffer, xBits, bitOffset, value);

    file.clear();
    file.seekp(static_cast<std::streamoff>(fileOffset));
    size_t bytesToWrite = (bitOffset + xBits > 8) ? 2 : 1;
    file.write(reinterpret_cast<char*>(buffer), bytesToWrite);

    return file.good();
}

int16_t DecodeSigned5Bits(uint32_t raw) {
    if (raw & 0x10) {
        return static_cast<int16_t>(static_cast<int32_t>(raw) - 32);
    }
    return static_cast<int16_t>(raw);
}

uint32_t EncodeSigned5Bits(int16_t value) {
    int32_t clamped = value;
    if (clamped < -15) clamped = -15;
    if (clamped > 15) clamped = 15;
    return static_cast<uint32_t>(clamped) & 0x1Fu;
}

int32_t DecodeSignedBits(uint32_t raw, int bitWidth) {
    if (bitWidth <= 0 || bitWidth >= 32) {
        return static_cast<int32_t>(raw);
    }
    uint32_t signBit = 1u << (bitWidth - 1);
    if (raw & signBit) {
        return static_cast<int32_t>(raw) - static_cast<int32_t>(1u << bitWidth);
    }
    return static_cast<int32_t>(raw);
}

uint32_t EncodeSignedBits(int32_t value, int bitWidth) {
    if (bitWidth <= 0 || bitWidth >= 32) {
        return static_cast<uint32_t>(value);
    }
    int32_t minValue = -static_cast<int32_t>(1u << (bitWidth - 1));
    int32_t maxValue = static_cast<int32_t>((1u << (bitWidth - 1)) - 1);
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;
    return static_cast<uint32_t>(value) & MaxValueForBits(bitWidth);
}