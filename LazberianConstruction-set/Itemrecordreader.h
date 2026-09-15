#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct ItemStats {
    uint16_t might = 0;
    uint16_t hexValue = 0;
    uint16_t accuracy = 0;
    uint16_t weight = 0;
    uint16_t maxRange = 0;
    uint16_t minRange = 0;
    uint16_t crit = 0;
    uint16_t uses = 0;
    uint16_t level = 0;
    uint32_t price = 0;

    uint16_t defense = 0;
    uint16_t speed = 0;
    uint16_t avoid = 0;
    uint16_t hit = 0;
    uint16_t magic = 0;
    uint16_t strength = 0;
    uint16_t rounds = 0;

    uint16_t fireRes = 0;
    uint16_t thunderRes = 0;
    uint16_t windRes = 0;
    uint16_t darkRes = 0;
    uint16_t holyRes = 0;

    uint16_t durabilityIndex = 0; // remember (s,a,b,c,d,e,f)
    uint16_t critAvoidPenalty = 0;

    uint16_t effectRateValue = 0;
    int32_t effectRateId = -1;

    // Do not string it, there is 3 "??" and it would break everything.
    std::vector<int> activeEffectIds;
};

ItemStats ReadItemStats(const std::string& isoPath, uint64_t dataOffset, uint64_t itemOffset);