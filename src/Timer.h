#pragma once

#include <cstdint>

class Timer
{
public:
    uint8_t TIMA;
    uint8_t TMA;
    uint8_t TAC;

    bool tick(uint16_t currentSysCounter, uint16_t previousSysCounter);

private:
    bool mDidOverflow = false;
};
