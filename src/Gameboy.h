#pragma once

#include "Clock.h"
#include "Cartridge.h"
#include "CPU.h"
#include "Timer.h"
#include "LCD.h"
#include "PPU.h"
#include "Joypad.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart, bool debug) : mFrameClock(FRAME_PERIOD),
                                                 mCart(std::move(cart)), mCPU(this, debug), mPPU(this, &(this->mLCD)) {}

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);

    static constexpr int wramSize = 8192;
    static constexpr int hramSize = 0xFFFE - 0xFF80 + 1;
    uint8_t wram[wramSize];
    uint8_t hram[hramSize];

    void requestInterrupt(CPU::InterruptSource source);

    void tickSystemCounter();

    const LCD::Color *getPixels() const;

    void tick(uint64_t deltaTime);
    void doTCycle();
    uint64_t getFrameClockTimeLeft();
    void setFastForward(bool enabled);
    void onKeyPress(int key);
    void onKeyRelease(int key);

private:
    Clock mFrameClock;
    Cartridge mCart;
    CPU mCPU;
    Timer mTimer;
    LCD mLCD;
    PPU mPPU;
    Joypad mJoypad;
    uint16_t mSysCounter = 0xABCC;

    static constexpr uint16_t VRAM_ADDR = 0x8000;
    static constexpr uint16_t SRAM_ADDR = 0xA000;
    static constexpr uint16_t WRAM0_ADDR = 0xC000;
    static constexpr uint16_t ECHO_ADDR = 0xE000;
    static constexpr uint16_t OAM_ADDR = 0xFE00;
    static constexpr uint16_t UNUSABLE_ADDR = 0xFEA0;
    static constexpr uint16_t JOYPAD_ADDR = 0xFF00;
    static constexpr uint16_t DIV_ADDR = 0xFF04;
    static constexpr uint16_t TIMA_ADDR = 0xFF05;
    static constexpr uint16_t TMA_ADDR = 0xFF06;
    static constexpr uint16_t TAC_ADDR = 0xFF07;
    static constexpr uint16_t IF_ADDR = 0xFF0F;
    static constexpr uint16_t LCDC_ADDR = 0xFF40;
    static constexpr uint16_t SCY_ADDR = 0xFF42;
    static constexpr uint16_t SCX_ADDR = 0xFF43;
    static constexpr uint16_t LY_ADDR = 0xFF44;
    static constexpr uint16_t DMA_ADDR = 0xFF46;
    static constexpr uint16_t HRAM_ADDR = 0xFF80;
    static constexpr uint16_t IE_ADDR = 0xFFFF;

    static constexpr uint64_t FRAME_PERIOD = 1e9 / 60; // nanoseconds
    static constexpr int CYCLES_PER_FRAME = 70224;
};
