#include "Gameboy.h"

#include <iostream>
#include <format>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address < VRAM_ADDR)
    {
        return mCart.readByte(address);
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
        uint8_t value = 0xFF;

        // If bit is 0, then is pressed
        if (mJoypad.selectButtons)
        {
            value &= ~Joypad::SEL_BUTTONS_BITMASK;
            if (mJoypad.keys[4])
                value &= ~Joypad::A_RIGHT_BITMASK;
            if (mJoypad.keys[5])
                value &= ~Joypad::B_LEFT_BITMASK;
            if (mJoypad.keys[6])
                value &= ~Joypad::START_DOWN_BITMASK;
            if (mJoypad.keys[7])
                value &= ~Joypad::SELECT_UP_BITMASK;
        }
        if (mJoypad.selectDPad)
        {
            value &= ~Joypad::SEL_DPAD_BITMASK;
            if (mJoypad.keys[3])
                value &= ~Joypad::A_RIGHT_BITMASK;
            if (mJoypad.keys[2])
                value &= ~Joypad::B_LEFT_BITMASK;
            if (mJoypad.keys[1])
                value &= ~Joypad::START_DOWN_BITMASK;
            if (mJoypad.keys[0])
                value &= ~Joypad::SELECT_UP_BITMASK;
        }

        return value;
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
    else if (address == NR10_ADDR)
    {
        return mAPU.NR10 | 0x80;
    }
    else if (address == NR11_ADDR)
    {
        return mAPU.NRx1[0] | 0x3F;
    }
    else if (address == NR12_ADDR)
    {
        return mAPU.NRx2[0];
    }
    else if (address == NR13_ADDR)
    {
        return 0xFF;
    }
    else if (address == NR14_ADDR)
    {
        return mAPU.NRx4[0] | 0xBF;
    }
    else if (address == NR21_ADDR)
    {
        return mAPU.NRx1[1] | 0x3F;
    }
    else if (address == NR22_ADDR)
    {
        return mAPU.NRx2[1];
    }
    else if (address == NR23_ADDR)
    {
        return 0xFF;
    }
    else if (address == NR24_ADDR)
    {
        return mAPU.NRx4[1] | 0xBF;
    }
    else if (address == LCDC_ADDR)
    {
        return mLCD.LCDC;
    }
    else if (address == STAT_ADDR)
    {
        return mLCD.STAT;
    }
    else if (address == SCY_ADDR)
    {
        return mLCD.SCY;
    }
    else if (address == SCX_ADDR)
    {
        return mLCD.SCX;
    }
    else if (address == LY_ADDR)
    {
        return mLCD.LY;
    }
    else if (address == LYC_ADDR)
    {
        return mLCD.LYC;
    }
    else if (address == BGP_ADDR)
    {
        return mLCD.BGP;
    }
    else if (address == OBP0_ADDR)
    {
        return mLCD.OBP0;
    }
    else if (address == OBP1_ADDR)
    {
        return mLCD.OBP1;
    }
    else if (address == WY_ADDR)
    {
        return mLCD.WY;
    }
    else if (address == WX_ADDR)
    {
        return mLCD.WX;
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
    if (address < VRAM_ADDR)
    {
        mCart.writeByte(address, value);
    }
    else if (address >= VRAM_ADDR && address < SRAM_ADDR)
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
    else if (address == JOYPAD_ADDR)
    {
        // If bit is 0, then select
        mJoypad.selectButtons = (value & Joypad::SEL_BUTTONS_BITMASK) ? false : true;
        mJoypad.selectDPad = (value & Joypad::SEL_DPAD_BITMASK) ? false : true;
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
    else if (address == NR10_ADDR)
    {
        mAPU.NR10 = value;
    }
    else if (address == NR11_ADDR)
    {
        mAPU.writeNRx1(1, value);
    }
    else if (address == NR12_ADDR)
    {
        mAPU.NRx2[0] = value;
    }
    else if (address == NR13_ADDR)
    {
        mAPU.NRx3[0] = value;
    }
    else if (address == NR14_ADDR)
    {
        mAPU.writeNRx4(1, value);
    }
    else if (address == NR21_ADDR)
    {
        mAPU.writeNRx1(2, value);
    }
    else if (address == NR22_ADDR)
    {
        mAPU.NRx2[1] = value;
    }
    else if (address == NR23_ADDR)
    {
        mAPU.NRx3[1] = value;
    }
    else if (address == NR24_ADDR)
    {
        mAPU.writeNRx4(2, value);
    }
    else if (address == LCDC_ADDR)
    {
        mLCD.LCDC = value;
    }
    else if (address == STAT_ADDR)
    {
        mLCD.STAT &= LCD::STAT_LYC_LY_BITMASK & LCD::STAT_MODE_BITMASK;
        mLCD.STAT |= (value & ~(LCD::STAT_LYC_LY_BITMASK & LCD::STAT_MODE_BITMASK));
    }
    else if (address == SCY_ADDR)
    {
        mLCD.SCY = value;
    }
    else if (address == SCX_ADDR)
    {
        mLCD.SCX = value;
    }
    else if (address == LYC_ADDR)
    {
        mLCD.LYC = value;
    }
    else if (address == DMA_ADDR)
    {
        mCPU.startDMATransfer(value);
    }
    else if (address == BGP_ADDR)
    {
        mLCD.BGP = value;
    }
    else if (address == OBP0_ADDR)
    {
        mLCD.OBP0 = value;
    }
    else if (address == OBP1_ADDR)
    {
        mLCD.OBP1 = value;
    }
    else if (address == WY_ADDR)
    {
        mLCD.WY = value;
    }
    else if (address == WX_ADDR)
    {
        mLCD.WX = value;
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

float Gameboy::getLeftAudioSample() const
{
    return mAPU.getLeftAudioSample();
}

float Gameboy::getRightAudioSample() const
{
    return mAPU.getRightAudioSample();
}

void Gameboy::doTCycle()
{
    mCPU.doTCycle();
    mPPU.doDot();
    mAPU.doTCycle();
}

void Gameboy::onKeyPress(int key)
{
    mJoypad.keys[key] = true;
}

void Gameboy::onKeyRelease(int key)
{
    mJoypad.keys[key] = false;
}
