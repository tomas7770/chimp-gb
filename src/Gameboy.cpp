#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Read byte from invalid address");
    }
    cycleInfo.push_back({address, debugRam[address], READ});

    return debugRam[address];
}

void Gameboy::writeByte(uint16_t address, uint8_t value)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Write byte to invalid address");
    }
    debugRam[address] = value;

    cycleInfo.push_back({address, value, WRITE});
}

void Gameboy::writeWord(uint16_t address, uint16_t value)
{
    if (address + 1 >= 1 << 16)
    {
        throw std::runtime_error("Write word to invalid address");
    }
    // Little-endian
    debugRam[address] = value & 0xFF;
    debugRam[address + 1] = value >> 8;

    cycleInfo.push_back({address, value, WRITE});
}

void Gameboy::doMCycle()
{
    cycleInfo.clear();
    mCPU.decodeExecutePrefetchOpcode();
}

CPU::CPUState Gameboy::getCPUState() const
{
    return mCPU.getState();
}

void Gameboy::setCPUState(CPU::CPUState state)
{
    mCPU.setState(state);
}
