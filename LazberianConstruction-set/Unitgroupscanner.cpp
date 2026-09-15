#include "UnitGroupScanner.h"
#include "UnitDataScanner.h"

#include <fstream>
#include <algorithm>

namespace {
    constexpr size_t UNIT_SIG_SIZE = 8;
    constexpr size_t ENDC_SIG_SIZE = 4;
    constexpr size_t STEP = 4;
	constexpr uint64_t MAX_GROUP_SCAN = 4ull * 1024 * 1024; // 4mb chunks at a time, RAM is expensive...

    bool MatchesSignature(const uint8_t* data, const uint8_t* signature, size_t length) {
        for (size_t i = 0; i < length; ++i) {
            if (data[i] != signature[i]) {
                return false;
            }
        }
        return true;
    }
}

UnitGroup ScanUnitGroup(const std::string& isoPath, uint64_t dataOffset, uint64_t dataSize, uint64_t groupStartOffset) {
    UnitGroup group;
    group.startOffset = groupStartOffset;

    if (groupStartOffset >= dataSize) {
        return group;
    }

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso.is_open()) {
        return group;
    }

    uint64_t remaining = dataSize - groupStartOffset;
    uint64_t toRead = std::min<uint64_t>(MAX_GROUP_SCAN, remaining);

    std::vector<uint8_t> buffer(static_cast<size_t>(toRead));
    iso.seekg(static_cast<std::streamoff>(dataOffset + groupStartOffset));
    iso.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    if (!iso && !iso.eof()) {
        return group;
    }

    size_t validBytes = buffer.size();
    if (validBytes < UNIT_SIG_SIZE) {
        return group;
    }

    size_t scanLimit = validBytes - UNIT_SIG_SIZE;
    for (size_t pos = 0; pos <= scanLimit; pos += STEP) {
        if (MatchesSignature(buffer.data() + pos, UNITSEEK_SIGNATURE, UNIT_SIG_SIZE)) {
            CharacterRecord record;
            record.offset = groupStartOffset + pos;
            group.characters.push_back(record);
        }
        else if (MatchesSignature(buffer.data() + pos, ENDC_SIGNATURE, ENDC_SIG_SIZE)) {
            group.endOffset = groupStartOffset + pos;
            return group;
        }
    }

    group.endOffset = groupStartOffset + validBytes;
    return group;
}