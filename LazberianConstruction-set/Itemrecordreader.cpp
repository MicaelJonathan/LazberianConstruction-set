#include "ItemRecordReader.h"
#include "GameTables.h"

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
    stats.hexValue = static_cast<uint16_t>(ReadBits(buffer.data() + 1, 4, 3));
    stats.accuracy = static_cast<uint16_t>(ReadBits(buffer.data() + 1, 7, 7));
    stats.weight = static_cast<uint16_t>(ReadBits(buffer.data() + 2, 5, 6));
    stats.maxRange = static_cast<uint16_t>(ReadBits(buffer.data() + 3, 5, 3));
    stats.minRange = static_cast<uint16_t>(ReadBits(buffer.data() + 4, 4, 0));
    stats.crit = static_cast<uint16_t>(ReadBits(buffer.data() + 5, 7, 0));
    stats.uses = static_cast<uint16_t>(ReadBits(buffer.data() + 5, 7, 7));
    stats.level = static_cast<uint16_t>(ReadBits(buffer.data() + 6, 6, 6));
    stats.price = ReadBits(buffer.data() + 8, 16, 0);

    stats.defense = static_cast<uint16_t>(ReadBits(buffer.data() + 12, 6, 0));
    stats.speed = static_cast<uint16_t>(ReadBits(buffer.data() + 13, 5, 3));
    stats.avoid = static_cast<uint16_t>(ReadBits(buffer.data() + 14, 8, 0));
    stats.hit = static_cast<uint16_t>(ReadBits(buffer.data() + 15, 8, 0));
    stats.magic = static_cast<uint16_t>(ReadBits(buffer.data() + 16, 5, 0));
    stats.strength = static_cast<uint16_t>(ReadBits(buffer.data() + 16, 5, 5));
    stats.rounds = static_cast<uint16_t>(ReadBits(buffer.data() + 17, 4, 2));

    stats.fireRes = static_cast<uint16_t>(ReadBits(buffer.data() + 17, 6, 6));
    stats.thunderRes = static_cast<uint16_t>(ReadBits(buffer.data() + 18, 6, 4));
    stats.windRes = static_cast<uint16_t>(ReadBits(buffer.data() + 19, 6, 2));
    stats.darkRes = static_cast<uint16_t>(ReadBits(buffer.data() + 20, 6, 0));
    stats.holyRes = static_cast<uint16_t>(ReadBits(buffer.data() + 20, 6, 6));

    stats.durabilityIndex = static_cast<uint16_t>(ReadBits(buffer.data() + 21, 3, 4));
    stats.critAvoidPenalty = static_cast<uint16_t>(ReadBits(buffer.data() + 21, 8, 7));

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