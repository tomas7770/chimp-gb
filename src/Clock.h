#pragma once

#include <cstdint>

// Time units in nanoseconds

class Clock
{
public:
    Clock(uint64_t period, uint64_t maxTimeAccum) : mPeriod(period), mMaxTimeAccum(maxTimeAccum) {}
    int tick(uint64_t deltaTime);

private:
    uint64_t mPeriod;
    uint64_t mMaxTimeAccum;

    uint64_t mTimeAccum = 0;
};
