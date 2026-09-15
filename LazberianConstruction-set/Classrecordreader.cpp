#include "ClassRecordReader.h"
#include "GameOffsets.h"
#include "GameTables.h"
#include "BitFieldIO.h"

#include <fstream>
#include <vector>

ClassRecord ReadClassRecord(const std::string& isoPath, uint64_t dataOffset, int languageUsed, uint32_t classId) {
    ClassRecord record;

    if (classId == 0) {
        return record;
    }

    uint64_t classOffset = ComputeClassOffset(languageUsed, classId);

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso.is_open()) {
        return record;
    }

    std::vector<uint8_t> buffer(CLASS_RECORD_SIZE, 0);
    iso.seekg(static_cast<std::streamoff>(dataOffset + classOffset));
    iso.read(reinterpret_cast<char*>(buffer.data()), CLASS_RECORD_SIZE);
    if (!iso && !iso.eof()) {
        return record;
    }

    record.hp = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_HP_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_HP_BIT_OFFSET));
    record.strength = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_STR_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_STR_BIT_OFFSET));
    record.speed = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_SPD_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_SPD_BIT_OFFSET));
    record.defense = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_DEF_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_DEF_BIT_OFFSET));
    record.magic = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_MAG_BYTE_OFFSET, CLASS_BASE_BIT_WIDTH, CLASS_MAG_BIT_OFFSET));
    record.move = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_MOVE_BYTE_OFFSET, CLASS_MOVE_BIT_WIDTH, CLASS_MOVE_BIT_OFFSET));
    record.experience = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_EXP_BYTE_OFFSET, CLASS_EXP_BIT_WIDTH, CLASS_EXP_BIT_OFFSET));

    uint32_t typeRaw = ReadBitsFromBuffer(buffer.data() + CLASS_TYPE_BYTE_OFFSET, CLASS_TYPE_BIT_WIDTH, CLASS_TYPE_BIT_OFFSET);
    record.typeIndex = -1;
    for (int bit = 0; bit < static_cast<int>(UnitTypes.size()); ++bit) {
        if (typeRaw == (1u << bit)) {
            record.typeIndex = bit;
            break;
        }
    }


    bool isFlier = (record.typeIndex >= 0 &&
        UnitTypes[record.typeIndex] == "Flier");
    bool mountedFlag = ReadBitsFromBuffer(buffer.data() + CLASS_MOUNTED_FLAG_BYTE_OFFSET, 1, CLASS_MOUNTED_FLAG_BIT_OFFSET) != 0;
    if (isFlier) {
        record.mountIndex = 2; // "flying" 
    }
    else if (mountedFlag) {
        record.mountIndex = 1; // "mounted"
    }
    else {
        record.mountIndex = 0; // "unmounted"
    }

    record.movementIndex = static_cast<int>(ReadBitsFromBuffer(buffer.data() + CLASS_MOVEMENT_BYTE_OFFSET, CLASS_MOVEMENT_BIT_WIDTH, CLASS_MOVEMENT_BIT_OFFSET));

    record.growthHp = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_GROWTH_HP_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_HP_BIT_OFFSET));
    record.growthStrength = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_GROWTH_STR_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_STR_BIT_OFFSET));
    record.growthSpeed = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_GROWTH_SPD_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_SPD_BIT_OFFSET));
    record.growthDefense = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_GROWTH_DEF_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_DEF_BIT_OFFSET));
    record.growthMagic = static_cast<uint16_t>(ReadBitsFromBuffer(buffer.data() + CLASS_GROWTH_MAG_BYTE_OFFSET, CLASS_GROWTH_BIT_WIDTH, CLASS_GROWTH_MAG_BIT_OFFSET));

    for (const auto& spec : ClassCapFieldSpecs) {
        uint32_t value = ReadBitsFromBuffer(buffer.data() + spec.byteOffset, CLASS_CAP_BIT_WIDTH, spec.bitOffset);
        record.caps[static_cast<size_t>(spec.category)] = static_cast<uint16_t>(value);
    }

    for (size_t skillId = 0; skillId < Skills.size(); ++skillId) {
        size_t byteIndex = CLASS_SKILL_FLAGS_BASE_OFFSET + skillId / 8;
        int bitIndex = static_cast<int>(skillId % 8);
        if (byteIndex >= buffer.size()) {
            break;
        }
        uint32_t flag = ReadBitsFromBuffer(buffer.data() + byteIndex, 1, bitIndex);
        if (flag && Skills[skillId] != "--") {
            record.activeSkillIds.push_back(static_cast<int>(skillId));
        }
    }

    return record;
}