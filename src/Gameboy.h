#pragma once

#include "Clock.h"
#include "Cartridge.h"
#include "CPU.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart) : mClock(this), mCart(std::move(cart)), mCPU(this) {}

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);

    static constexpr int wramSize = 8192;
    uint8_t wram[wramSize];

    void tick(uint64_t deltaTime);
    void doTCycle();

private:
    Clock mClock;
    Cartridge mCart;
    CPU mCPU;

    static constexpr uint16_t VRAM_ADDR = 0x8000;
    static constexpr uint16_t WRAM0_ADDR = 0xC000;
    static constexpr uint16_t ECHO_ADDR = 0xE000;
    static constexpr uint16_t OAM_ADDR = 0xFE00;
};
