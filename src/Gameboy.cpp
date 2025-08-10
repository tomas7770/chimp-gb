#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Read byte from invalid address");
    }

    if (address < VRAM_ADDR)
    {
        return mCart.getData().at(address);
    }
    else if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        return wram[address - WRAM0_ADDR];
    }
    else if (address >= ECHO_ADDR && address < OAM_ADDR)
    {
        return wram[address - ECHO_ADDR];
    }
    else if (address == IF_ADDR)
    {
        return mCPU.IF;
    }
    else if (address >= HRAM_ADDR && address < IE_ADDR)
    {
        return hram[address - HRAM_ADDR];
    }
    else if (address == IE_ADDR)
    {
        return mCPU.IE;
    }
    // DEBUG
    else if (address == 0xFF44)
    {
        return 0x90;
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
    else if (address == IF_ADDR)
    {
        mCPU.IF = value;
    }
    else if (address >= HRAM_ADDR && address < IE_ADDR)
    {
        hram[address - HRAM_ADDR] = value;
    }
    else if (address == IE_ADDR)
    {
        mCPU.IE = value;
    }
    // DEBUG/TESTING
    else if (address == 0xFF01)
    {
        mDebugChar = value;
    }
    else if (address == 0xFF02 && value == 0x81)
    {
        std::cout << mDebugChar;
    }
}

void Gameboy::tick(uint64_t deltaTime)
{
    int numTicks = mClock.tick(deltaTime);
    for (int i = 0; i < numTicks; i++)
    {
        doTCycle();
    }
}

void Gameboy::doTCycle()
{
    mCPU.doTCycle();
}
