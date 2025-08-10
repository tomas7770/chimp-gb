#pragma once

#include "Clock.h"
#include "Cartridge.h"
#include "CPU.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart, bool debug) : mClock(CLOCK_CYCLE_PERIOD, CLOCK_MAX_TIME_ACCUM),
                                                 mCart(std::move(cart)), mCPU(this, debug) {}

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);

    static constexpr int wramSize = 8192;
    static constexpr int hramSize = 0xFFFE - 0xFF80 + 1;
    uint8_t wram[wramSize];
    uint8_t hram[hramSize];

    void tick(uint64_t deltaTime);
    void doTCycle();

private:
    Clock mClock;
    Cartridge mCart;
    CPU mCPU;
    // DEBUG/TESTING
    uint8_t mDebugChar;

    static constexpr uint16_t VRAM_ADDR = 0x8000;
    static constexpr uint16_t WRAM0_ADDR = 0xC000;
    static constexpr uint16_t ECHO_ADDR = 0xE000;
    static constexpr uint16_t OAM_ADDR = 0xFE00;
    static constexpr uint16_t IF_ADDR = 0xFF0F;
    static constexpr uint16_t HRAM_ADDR = 0xFF80;
    static constexpr uint16_t IE_ADDR = 0xFFFF;

    static constexpr uint64_t CLOCK_CYCLE_PERIOD = 1e9 / 4194304;
    static constexpr uint64_t CLOCK_MAX_TIME_ACCUM = 1e9 / 15;
};
