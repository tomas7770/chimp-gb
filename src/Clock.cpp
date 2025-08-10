#include "Clock.h"

int Clock::tick(uint64_t deltaTime)
{
    mTimeAccum += deltaTime;
    if (mTimeAccum > mMaxTimeAccum)
        mTimeAccum = mMaxTimeAccum;
    int numTicks = 0;
    while (mTimeAccum >= mPeriod)
    {
        numTicks++;
        mTimeAccum -= mPeriod;
    }
    return numTicks;
}
