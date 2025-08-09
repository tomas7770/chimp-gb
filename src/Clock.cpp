#include "Clock.h"
#include "Gameboy.h"

void Clock::tick(uint64_t deltaTime)
{
    mTimeAccum += deltaTime;
    if (mTimeAccum > MAX_TIME_ACCUM)
        mTimeAccum = MAX_TIME_ACCUM;
    while (mTimeAccum >= CYCLE_PERIOD)
    {
        mGameboy->doTCycle();
        mTimeAccum -= CYCLE_PERIOD;
    }
}
