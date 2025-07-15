#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Read byte from invalid address");
    }
    lastAddress = address;
    lastValue = debugRam[address];
    lastAccessType = READ;

    return debugRam[address];
}

void Gameboy::writeByte(uint16_t address, uint8_t value)
{
    if (address >= 1 << 16)
    {
        throw std::runtime_error("Write byte to invalid address");
    }
    debugRam[address] = value;

    lastAddress = address;
    lastValue = value;
    lastAccessType = WRITE;
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

    lastAddress = address;
    lastValue = value;
    lastAccessType = WRITE;
}

void Gameboy::doMCycle()
{
    lastAccessType = NONE;
    mCPU.fetchDecodeExecuteOpcode();
}

CPU::CPUState Gameboy::getCPUState() const
{
    return mCPU.getState();
}
