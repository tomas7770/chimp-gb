#pragma once

#include "Cartridge.h"
#include "CPU.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart) : mCart(std::move(cart)) {}

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);
    void writeWord(uint16_t address, uint16_t value);

    static constexpr int wramSize = 8192;
    uint8_t wram[wramSize];

    // DEBUG/TESTING
    void doMCycle();
    uint8_t debugRam[1 << 16];
    enum AccessType
    {
        READ,
        WRITE,
        NONE
    };
    struct MemoryAccess
    {
        uint16_t address;
        uint16_t value;
        AccessType accessType;
    };
    std::vector<MemoryAccess> cycleInfo;

    CPU::CPUState getCPUState() const;

private:
    Cartridge mCart;
    CPU mCPU;
};
