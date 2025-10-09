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
    enum SystemType
    {
        DMG,
        CGB,
    };

    Gameboy(const Cartridge &cart, bool debug, SystemType systemType)
        : mSystemType(systemType), mCart(std::move(cart)), mCPU(this, debug), mPPU(this, &(this->mLCD)) {}

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);

    static constexpr int wramSize = 32768;
    static constexpr int hramSize = 0xFFFE - 0xFF80 + 1;
    static constexpr int dmgBootRomSize = 0x100;
    static constexpr int cgbBootRomSize = 0x900;
    uint8_t wram[wramSize];
    uint8_t hram[hramSize];
    uint8_t bootRom[cgbBootRomSize];

    unsigned int tCycleCounter = 0;

    void requestInterrupt(CPU::InterruptSource source);

    void tickSystemCounter();

    const LCD::Color *getPixels() const;
    void computeAudioSamples();
    float getLeftAudioSample() const;
    float getRightAudioSample() const;

    void doTCycle();
    void onKeyPress(int key);
    void onKeyRelease(int key);

    Cartridge &getCart();

    void setDrawCallback(void (*drawCallback)(void *), void *userdata);
    void setBootRom(std::istream &dataStream);
    void simulateBootRom();

    SystemType getSystemType();
    bool inDMGMode();
    bool inCGBMode();

    bool inHBlank();

    static constexpr int CYCLES_PER_FRAME = 70224;
    static constexpr int CLOCK_RATE = 4194304;

    static constexpr int CPU_CYCLE_DIV = 4;
    static constexpr int APU_CYCLE_DIV = 2;

private:
    SystemType mSystemType;
    Cartridge mCart;
    CPU mCPU;
    Timer mTimer;
    LCD mLCD;
    PPU mPPU;
    APU mAPU;
    Joypad mJoypad;
    bool mBootRomFinished = true;

    uint16_t mSysCounter = 0xABCC;
    uint8_t mKEY0 = 0;
    int mWRAMBank = 1;

    static constexpr uint16_t WRAM_BANK_SIZE = (1 << 12);

    static constexpr uint8_t DMG_MODE_FALLBACK_PALETTE[8] = {0xFF, 0xFF, 0xEF, 0xBD, 0xE7, 0x9C, 0x00, 0x80};

    static constexpr uint16_t BOOT_ROM_END_ADDR = 0x00FF;
    static constexpr uint16_t BOOT_ROM_CGB_START_ADDR = 0x0200;
    static constexpr uint16_t BOOT_ROM_CGB_END_ADDR = 0x08FF;
    static constexpr uint16_t VRAM_ADDR = 0x8000;
    static constexpr uint16_t SRAM_ADDR = 0xA000;
    static constexpr uint16_t WRAM0_ADDR = 0xC000;
    static constexpr uint16_t WRAM1_ADDR = 0xD000;
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
    static constexpr uint16_t NR50_ADDR = 0xFF24;
    static constexpr uint16_t NR51_ADDR = 0xFF25;
    static constexpr uint16_t NR52_ADDR = 0xFF26;
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
    static constexpr uint16_t KEY0_ADDR = 0xFF4C;
    static constexpr uint16_t KEY1_ADDR = 0xFF4D;
    static constexpr uint16_t VBK_ADDR = 0xFF4F;
    static constexpr uint16_t BANK_ADDR = 0xFF50;
    static constexpr uint16_t HDMA1_ADDR = 0xFF51;
    static constexpr uint16_t HDMA2_ADDR = 0xFF52;
    static constexpr uint16_t HDMA3_ADDR = 0xFF53;
    static constexpr uint16_t HDMA4_ADDR = 0xFF54;
    static constexpr uint16_t HDMA5_ADDR = 0xFF55;
    static constexpr uint16_t BCPS_ADDR = 0xFF68;
    static constexpr uint16_t BCPD_ADDR = 0xFF69;
    static constexpr uint16_t OCPS_ADDR = 0xFF6A;
    static constexpr uint16_t OCPD_ADDR = 0xFF6B;
    static constexpr uint16_t SVBK_ADDR = 0xFF70;
    static constexpr uint16_t HRAM_ADDR = 0xFF80;
    static constexpr uint16_t IE_ADDR = 0xFFFF;
};
