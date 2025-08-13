#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address < VRAM_ADDR)
    {
        return mCart.getData().at(address);
    }
    else if (address >= VRAM_ADDR && address < SRAM_ADDR)
    {
        // if (mPPU.getMode() == 3)
        // {
        //     return 0xFF; // garbage
        // }
        // else
        // {
        //     return mPPU.vram[address - VRAM_ADDR];
        // }
        return mPPU.vram[address - VRAM_ADDR];
    }
    else if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        return wram[address - WRAM0_ADDR];
    }
    else if (address >= ECHO_ADDR && address < OAM_ADDR)
    {
        return wram[address - ECHO_ADDR];
    }
    else if (address >= OAM_ADDR && address < UNUSABLE_ADDR)
    {
        // if (mPPU.getMode() == 0 || mPPU.getMode() == 1)
        // {
        //     return mPPU.oam[address - OAM_ADDR];
        // }
        // else
        // {
        //     return 0xFF; // garbage
        // }
        return mPPU.oam[address - OAM_ADDR];
    }
    // TODO implement unusable memory
    else if (address == JOYPAD_ADDR)
    {
        // TODO (0xFF means all buttons released, so it'll be the default until joypad is implemented)
        return 0xFF;
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
    else if (address == LCDC_ADDR)
    {
        return mLCD.LCDC;
    }
    else if (address == LY_ADDR)
    {
        return mLCD.LY;
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
    if (address >= VRAM_ADDR && address < SRAM_ADDR)
    {
        // if (mPPU.getMode() != 3)
        // {
        //     mPPU.vram[address - VRAM_ADDR] = value;
        // }
        mPPU.vram[address - VRAM_ADDR] = value;
    }
    else if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        wram[address - WRAM0_ADDR] = value;
    }
    else if (address >= ECHO_ADDR && address < OAM_ADDR)
    {
        wram[address - ECHO_ADDR] = value;
    }
    else if (address >= OAM_ADDR && address < UNUSABLE_ADDR)
    {
        // if (mPPU.getMode() == 0 || mPPU.getMode() == 1)
        // {
        //     mPPU.oam[address - OAM_ADDR] = value;
        // }
        mPPU.oam[address - OAM_ADDR] = value;
    }
    // TODO implement unusable memory
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
    else if (address == LCDC_ADDR)
    {
        mLCD.LCDC = value;
    }
    else if (address == DMA_ADDR)
    {
        mCPU.startDMATransfer(value);
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

void Gameboy::requestInterrupt(CPU::InterruptSource source)
{
    mCPU.requestInterrupt(source);
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
    bool doTick = mFrameClock.tick(deltaTime);
    if (doTick)
    {
        for (int i = 0; i < CYCLES_PER_FRAME; i++)
        {
            doTCycle();
        }
    }
}

void Gameboy::doTCycle()
{
    mCPU.doTCycle();
    mPPU.doDot();
}

uint64_t Gameboy::getFrameClockTimeLeft()
{
    return mFrameClock.getTimeLeft();
}
