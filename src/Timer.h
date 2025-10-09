#pragma once

#include <cstdint>

class Timer
{
public:
    uint8_t TIMA = 0x00;
    uint8_t TMA = 0x00;
    uint8_t TAC = 0xF8;

    bool tick(uint16_t currentSysCounter, uint16_t previousSysCounter);

private:
    bool mDidOverflow = false;
};
