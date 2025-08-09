#pragma once

#include <cstdint>

class Gameboy;

// Time units in nanoseconds

class Clock
{
public:
    Clock(Gameboy *gameboy) : mGameboy(gameboy) {}
    void tick(uint64_t deltaTime);

private:
    Gameboy *mGameboy;

    uint64_t mTimeAccum = 0;

    static constexpr uint64_t CYCLE_PERIOD = 1e9 / 4194304;
    static constexpr uint64_t MAX_TIME_ACCUM = 1e9 / 15;
};
