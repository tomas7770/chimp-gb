#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Read byte from invalid address");
    }

    if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        return wram[address - WRAM0_ADDR];
    }
    else if (address >= ECHO_ADDR && address < OAM_ADDR)
    {
        return wram[address - ECHO_ADDR];
    }

    return 0;
}

void Gameboy::writeByte(uint16_t address, uint8_t value)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Write byte to invalid address");
    }

    if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        wram[address - WRAM0_ADDR] = value;
    }
    else if (address >= ECHO_ADDR && address < OAM_ADDR)
    {
        wram[address - ECHO_ADDR] = value;
    }
}

void Gameboy::doMCycle()
{
    mCPU.doMCycle();
}
