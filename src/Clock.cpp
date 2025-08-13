#include "Clock.h"

bool Clock::tick(uint64_t deltaTime)
{
    mTimeAccum += deltaTime;
    if (mTimeAccum >= mPeriod)
    {
        mTimeAccum -= mPeriod;
        if (mTimeAccum >= mPeriod)
        {
            mTimeAccum = mPeriod;
        }
        return true;
    }
    return false;
}

uint64_t Clock::getTimeLeft()
{
    return mPeriod - mTimeAccum;
}
