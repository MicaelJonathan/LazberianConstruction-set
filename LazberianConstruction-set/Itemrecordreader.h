#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct ItemStats {
    uint16_t might = 0;
    int32_t hexValue = 0;
    uint16_t accuracy = 0;
    int32_t weight = 0;
    int32_t maxRange = 0;
    int32_t minRange = 0;
    int32_t crit = 0;
    uint16_t uses = 0;
    uint16_t level = 0;
    int32_t price = 0;

    int32_t defense = 0;
    int32_t speed = 0;
    int32_t avoid = 0;
    int32_t hit = 0;
    int32_t magic = 0;
    int32_t strength = 0;
    int32_t rounds = 0;

    int32_t fireRes = 0;
    int32_t thunderRes = 0;
    int32_t windRes = 0;
    int32_t darkRes = 0;
    int32_t holyRes = 0;

    uint16_t durabilityIndex = 0; // remember (s,a,b,c,d,e,f)
    int32_t critAvoidPenalty = 0;

    uint16_t effectRateValue = 0;
    int32_t effectRateId = -1;

    // Do not string it, there is 3 "??" and it would break everything.
    std::vector<int> activeEffectIds;
};

ItemStats ReadItemStats(const std::string& isoPath, uint64_t dataOffset, uint64_t itemOffset);