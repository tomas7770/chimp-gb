#pragma once

#include <cstdint>

class Clock
{
public:
    Clock(uint64_t period) : mPeriod(period) {}
    bool tick(uint64_t deltaTime);
    uint64_t getTimeLeft();

private:
    uint64_t mPeriod;

    uint64_t mTimeAccum = 0;
};
