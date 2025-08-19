#pragma once

#include "Cartridge.h"
#include "CPU.h"
#include "Timer.h"
#include "LCD.h"
#include "PPU.h"
#include "APU.h"
#include "Joypad.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart, bool debug) : mCart(std::move(cart)), mCPU(this, debug), mPPU(this, &(this->mLCD)) {}

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);

    static constexpr int wramSize = 8192;
    static constexpr int hramSize = 0xFFFE - 0xFF80 + 1;
    uint8_t wram[wramSize];
    uint8_t hram[hramSize];

    void requestInterrupt(CPU::InterruptSource source);

    void tickSystemCounter();

    const LCD::Color *getPixels() const;
    float getLeftAudioSample() const;
    float getRightAudioSample() const;

    void doTCycle();
    void onKeyPress(int key);
    void onKeyRelease(int key);

    static constexpr int CYCLES_PER_FRAME = 70224;
    static constexpr int CLOCK_RATE = 4194304;

private:
    Cartridge mCart;
    CPU mCPU;
    Timer mTimer;
    LCD mLCD;
    PPU mPPU;
    APU mAPU;
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
    static constexpr uint16_t NR10_ADDR = 0xFF10;
    static constexpr uint16_t NR11_ADDR = 0xFF11;
    static constexpr uint16_t NR12_ADDR = 0xFF12;
    static constexpr uint16_t NR13_ADDR = 0xFF13;
    static constexpr uint16_t NR14_ADDR = 0xFF14;
    static constexpr uint16_t NR21_ADDR = 0xFF16;
    static constexpr uint16_t NR22_ADDR = 0xFF17;
    static constexpr uint16_t NR23_ADDR = 0xFF18;
    static constexpr uint16_t NR24_ADDR = 0xFF19;
    static constexpr uint16_t NR30_ADDR = 0xFF1A;
    static constexpr uint16_t NR31_ADDR = 0xFF1B;
    static constexpr uint16_t NR32_ADDR = 0xFF1C;
    static constexpr uint16_t NR33_ADDR = 0xFF1D;
    static constexpr uint16_t NR34_ADDR = 0xFF1E;
    static constexpr uint16_t NR41_ADDR = 0xFF20;
    static constexpr uint16_t NR42_ADDR = 0xFF21;
    static constexpr uint16_t NR43_ADDR = 0xFF22;
    static constexpr uint16_t NR44_ADDR = 0xFF23;
    static constexpr uint16_t WAVE_RAM_START_ADDR = 0xFF30;
    static constexpr uint16_t WAVE_RAM_END_ADDR = 0xFF3F;
    static constexpr uint16_t LCDC_ADDR = 0xFF40;
    static constexpr uint16_t STAT_ADDR = 0xFF41;
    static constexpr uint16_t SCY_ADDR = 0xFF42;
    static constexpr uint16_t SCX_ADDR = 0xFF43;
    static constexpr uint16_t LY_ADDR = 0xFF44;
    static constexpr uint16_t LYC_ADDR = 0xFF45;
    static constexpr uint16_t DMA_ADDR = 0xFF46;
    static constexpr uint16_t BGP_ADDR = 0xFF47;
    static constexpr uint16_t OBP0_ADDR = 0xFF48;
    static constexpr uint16_t OBP1_ADDR = 0xFF49;
    static constexpr uint16_t WY_ADDR = 0xFF4A;
    static constexpr uint16_t WX_ADDR = 0xFF4B;
    static constexpr uint16_t HRAM_ADDR = 0xFF80;
    static constexpr uint16_t IE_ADDR = 0xFFFF;
};
