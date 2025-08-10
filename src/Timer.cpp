#include "Timer.h"

bool Timer::tick(uint16_t currentSysCounter, uint16_t previousSysCounter)
{
    // Emulate Gameboy's timer circuit
    bool doRequestInterrupt = false;
    // Timer reset and interrupt are delayed by 1 M-cycle
    if (mDidOverflow)
    {
        mDidOverflow = false;
        TIMA = TMA;
        doRequestInterrupt = true;
    }

    int selectedBit;
    switch (TAC & 0b11)
    {
    default:
    case 0:
        selectedBit = 7;
        break;
    case 1:
        selectedBit = 1;
        break;
    case 2:
        selectedBit = 3;
        break;
    case 3:
        selectedBit = 5;
        break;
    }

    int currentValue = (currentSysCounter >> selectedBit) & 1 & (TAC >> 2);
    int previousValue = (previousSysCounter >> selectedBit) & 1 & (TAC >> 2);

    if (previousValue == 1 && currentValue == 0)
    {
        // Falling edge detected, tick timer
        if (TIMA == 0xFF)
        {
            // Will overflow
            mDidOverflow = true;
        }
        TIMA++;
    }

    return doRequestInterrupt;
}
