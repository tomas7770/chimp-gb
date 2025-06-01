#pragma once

#include <cstdint>
#include <memory>

class Gameboy;

class CPU
{
public:
private:
    std::shared_ptr<Gameboy> mGameboy;

    // AF
    uint8_t mRegA;
    uint8_t mRegF;
    // BC
    uint8_t mRegB;
    uint8_t mRegC;
    // DE
    uint8_t mRegD;
    uint8_t mRegE;
    // HL
    uint8_t mRegH;
    uint8_t mRegL;
    // SP and PC
    uint16_t mSP;
    uint16_t mPC;
};
