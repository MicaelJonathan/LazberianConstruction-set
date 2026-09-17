#include "ItemRecordReader.h"
#include "GameTables.h"
#include "BitFieldIO.h"

#include <fstream>
#include <vector>

namespace {
    uint32_t ReadBits(const uint8_t* data, int xBits, int bitOffset) {
        uint32_t mask = (xBits >= 8) ? 0xFFu : ((1u << xBits) - 1u);
        uint32_t value = ((mask << bitOffset) & 0xFFu & data[0]) >> bitOffset;
        if (bitOffset + xBits > 8) {
            uint32_t highMask = (1u << (bitOffset + xBits - 8)) - 1u;
            value += (data[1] & highMask) << (8 - bitOffset);
        }
        return value;
    }
}

ItemStats ReadItemStats(const std::string& isoPath, uint64_t dataOffset, uint64_t itemOffset) {
    ItemStats stats;

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso.is_open()) {
        return stats;
    }

    constexpr size_t WINDOW = 56;
    std::vector<uint8_t> buffer(WINDOW, 0);
    iso.seekg(static_cast<std::streamoff>(dataOffset + itemOffset));
    iso.read(reinterpret_cast<char*>(buffer.data()), WINDOW);
    if (!iso && !iso.eof()) {
        return stats;
    }

    stats.might = static_cast<uint16_t>(ReadBits(buffer.data() + 0, 6, 5));
    stats.hexValue = DecodeSignedBits(ReadBits(buffer.data() + 1, 4, 3), 4);
    stats.accuracy = static_cast<uint16_t>(ReadBits(buffer.data() + 1, 7, 7));
    stats.weight = DecodeSignedBits(ReadBits(buffer.data() + 2, 5, 6), 5);
    stats.maxRange = DecodeSignedBits(ReadBits(buffer.data() + 3, 5, 3), 5);
    stats.minRange = DecodeSignedBits(ReadBits(buffer.data() + 4, 4, 0), 4);
    stats.crit = DecodeSignedBits(ReadBits(buffer.data() + 5, 7, 0), 7);
    stats.uses = static_cast<uint16_t>(ReadBits(buffer.data() + 5, 7, 7));
    stats.level = static_cast<uint16_t>(ReadBits(buffer.data() + 6, 6, 6));
    stats.price = DecodeSignedBits(ReadBits(buffer.data() + 8, 16, 0), 16);

    stats.defense = DecodeSignedBits(ReadBits(buffer.data() + 12, 6, 0), 6);
    stats.speed = DecodeSignedBits(ReadBits(buffer.data() + 13, 5, 3), 5);
    stats.avoid = DecodeSignedBits(ReadBits(buffer.data() + 14, 8, 0), 8);
    stats.hit = DecodeSignedBits(ReadBits(buffer.data() + 15, 8, 0), 8);
    stats.magic = DecodeSignedBits(ReadBits(buffer.data() + 16, 5, 0), 5);
    stats.strength = DecodeSignedBits(ReadBits(buffer.data() + 16, 5, 5), 5);
    stats.rounds = DecodeSignedBits(ReadBits(buffer.data() + 17, 4, 2), 4);

    stats.fireRes = DecodeSignedBits(ReadBits(buffer.data() + 17, 6, 6), 6);
    stats.thunderRes = DecodeSignedBits(ReadBits(buffer.data() + 18, 6, 4), 6);
    stats.windRes = DecodeSignedBits(ReadBits(buffer.data() + 19, 6, 2), 6);
    stats.darkRes = DecodeSignedBits(ReadBits(buffer.data() + 20, 6, 0), 6);
    stats.holyRes = DecodeSignedBits(ReadBits(buffer.data() + 20, 6, 6), 6);

    stats.durabilityIndex = static_cast<uint16_t>(ReadBits(buffer.data() + 21, 3, 4));
    stats.critAvoidPenalty = DecodeSignedBits(ReadBits(buffer.data() + 21, 8, 7), 8);

    stats.effectRateValue = static_cast<uint16_t>(ReadBits(buffer.data() + 26, 7, 0));
    uint16_t rawEffectRateId = static_cast<uint16_t>(ReadBits(buffer.data() + 27, 8, 0));
    stats.effectRateId = (rawEffectRateId >= 100) ? static_cast<int32_t>(rawEffectRateId) - 100 : -1;


    for (size_t effId = 0; effId < ItemEffects.size(); ++effId) {
        size_t byteIndex = 28 + effId / 8;
        int bitIndex = static_cast<int>(effId % 8);
        if (byteIndex >= buffer.size()) {
            break;
        }
        uint32_t flag = ReadBits(buffer.data() + byteIndex, 1, bitIndex);
        if (flag && ItemEffects[effId] != "--") {
            stats.activeEffectIds.push_back(static_cast<int>(effId));
        }
    }

    return stats;
}