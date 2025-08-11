#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
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
    else if (address == DIV_ADDR)
    {
        return (mSysCounter >> 6) & 0xFF;
    }
    else if (address == TIMA_ADDR)
    {
        return mTimer.TIMA;
    }
    else if (address == TMA_ADDR)
    {
        return mTimer.TMA;
    }
    else if (address == TAC_ADDR)
    {
        return mTimer.TAC;
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

    return 0;
}

void Gameboy::writeByte(uint16_t address, uint8_t value)
{
    if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        wram[address - WRAM0_ADDR] = value;
    }
    else if (address >= ECHO_ADDR && address < OAM_ADDR)
    {
        wram[address - ECHO_ADDR] = value;
    }
    else if (address == DIV_ADDR)
    {
        // The whole system internal counter is set to 0
        mSysCounter = 0;
    }
    else if (address == TIMA_ADDR)
    {
        mTimer.TIMA = value;
    }
    else if (address == TMA_ADDR)
    {
        mTimer.TMA = value;
    }
    else if (address == TAC_ADDR)
    {
        mTimer.TAC = value;
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
}

void Gameboy::tickSystemCounter()
{
    mSysCounter++;
    if (mTimer.tick(mSysCounter, mSysCounter - 1))
    {
        mCPU.requestInterrupt(CPU::InterruptSource::Timer);
    }
}

const LCD::Color* Gameboy::getPixels() const
{
    return mLCD.pixels;
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
