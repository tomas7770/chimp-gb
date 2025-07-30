#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Read byte from invalid address");
    }

    if (cycleInfo.accessType != NONE)
    {
        throw std::runtime_error("Multiple memory accesses in single M-cycle");
    }
    cycleInfo = {address, debugRam[address], READ};

    return debugRam[address];
}

void Gameboy::writeByte(uint16_t address, uint8_t value)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Write byte to invalid address");
    }
    debugRam[address] = value;

    if (cycleInfo.accessType != NONE)
    {
        throw std::runtime_error("Multiple memory accesses in single M-cycle");
    }
    cycleInfo = {address, value, WRITE};
}

void Gameboy::doMCycle()
{
    cycleInfo = {0, 0, NONE};
    mCPU.doMCycle();
}

CPU::CPUState Gameboy::getCPUState() const
{
    return mCPU.getState();
}

void Gameboy::setCPUState(CPU::CPUState state)
{
    mCPU.setState(state);
}
