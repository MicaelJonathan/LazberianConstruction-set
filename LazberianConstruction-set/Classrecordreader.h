#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "ClassFieldSpecs.h"

struct ClassRecord {
    // HP/Strength/Speed/Defense/Magic 0-31
    uint16_t hp = 0;
    uint16_t strength = 0;
    uint16_t speed = 0;
    uint16_t defense = 0;
    uint16_t magic = 0;
    uint16_t move = 0;
    uint16_t experience = 0;

    int mountIndex = 0;    
    int typeIndex = -1;    
    int movementIndex = 0; 

    uint16_t growthHp = 0;
    uint16_t growthStrength = 0;
    uint16_t growthSpeed = 0;
    uint16_t growthDefense = 0;
    uint16_t growthMagic = 0;

    uint16_t caps[static_cast<size_t>(WeaponCategory::Count)] = {};

    std::vector<int> activeSkillIds;
};

ClassRecord ReadClassRecord(const std::string& isoPath, uint64_t dataOffset, int languageUsed, uint32_t classId);