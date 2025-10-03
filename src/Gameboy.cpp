#include "Gameboy.h"

#include <iostream>
#include <format>
#include <cstring>

uint8_t Gameboy::readByte(uint16_t address)
{
    if (address < VRAM_ADDR)
    {
        if (!(mBootRomFinished) && (address <= BOOT_ROM_END_ADDR || (mSystemType == CGB && address >= BOOT_ROM_CGB_START_ADDR && address <= BOOT_ROM_CGB_END_ADDR)))
        {
            return bootRom[address];
        }
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
        int finalAddress = address - VRAM_ADDR;
        if (mPPU.VRAMBank)
        {
            finalAddress += PPU::VRAM_BANK_SIZE;
        }
        return mPPU.vram[finalAddress];
    }
    else if (address >= SRAM_ADDR && address < WRAM0_ADDR)
    {
        return mCart.readByte(address);
    }
    else if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        int finalAddress = address - WRAM0_ADDR;
        if (address >= WRAM1_ADDR)
        {
            finalAddress += WRAM_BANK_SIZE * (mWRAMBank - 1);
        }
        return wram[finalAddress];
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
                value &= ~Joypad::SELECT_UP_BITMASK;
            if (mJoypad.keys[7])
                value &= ~Joypad::START_DOWN_BITMASK;
        }
        if (mJoypad.selectDPad)
        {
            value &= ~Joypad::SEL_DPAD_BITMASK;
            if (mJoypad.keys[0])
                value &= ~Joypad::A_RIGHT_BITMASK;
            if (mJoypad.keys[1])
                value &= ~Joypad::B_LEFT_BITMASK;
            if (mJoypad.keys[2])
                value &= ~Joypad::SELECT_UP_BITMASK;
            if (mJoypad.keys[3])
                value &= ~Joypad::START_DOWN_BITMASK;
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
    else if (address == NR30_ADDR)
    {
        return mAPU.NR30 | 0x7F;
    }
    else if (address == NR31_ADDR)
    {
        return 0xFF;
    }
    else if (address == NR32_ADDR)
    {
        return mAPU.NRx2[2] | 0x9F;
    }
    else if (address == NR33_ADDR)
    {
        return 0xFF;
    }
    else if (address == NR34_ADDR)
    {
        return mAPU.NRx4[2] | 0xBF;
    }
    else if (address == NR41_ADDR)
    {
        return 0xFF;
    }
    else if (address == NR42_ADDR)
    {
        return mAPU.NRx2[3];
    }
    else if (address == NR43_ADDR)
    {
        return mAPU.NRx3[3];
    }
    else if (address == NR44_ADDR)
    {
        return mAPU.NRx4[3] | 0xBF;
    }
    else if (address == NR50_ADDR)
    {
        return mAPU.NR50;
    }
    else if (address == NR51_ADDR)
    {
        return mAPU.NR51;
    }
    else if (address == NR52_ADDR)
    {
        return mAPU.readNR52();
    }
    else if (address >= WAVE_RAM_START_ADDR && address <= WAVE_RAM_END_ADDR)
    {
        return mAPU.waveRam[address - WAVE_RAM_START_ADDR];
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
    else if (address == KEY0_ADDR)
    {
        return mKEY0;
    }
    else if (address == KEY1_ADDR && inCGBMode())
    {
        return (mCPU.isDoubleSpeed() ? (1 << 7) : 0) | 0b01111110 | (mCPU.armedSpeedSwitch ? 1 : 0);
    }
    else if (address == VBK_ADDR)
    {
        return mPPU.VRAMBank & 0xFE;
    }
    else if (address == HDMA5_ADDR && inCGBMode())
    {
        return mCPU.getDMALengthLeft();
    }
    else if (address == BCPS_ADDR)
    {
        return mLCD.BCPS;
    }
    else if (address == BCPD_ADDR)
    {
        return mLCD.colorBGPaletteMemory[mLCD.BCPS & 63];
    }
    else if (address == OCPS_ADDR)
    {
        return mLCD.OCPS;
    }
    else if (address == OCPD_ADDR)
    {
        return mLCD.colorOBJPaletteMemory[mLCD.OCPS & 63];
    }
    else if (address == SVBK_ADDR)
    {
        return mWRAMBank;
    }
    else if (address >= HRAM_ADDR && address < IE_ADDR)
    {
        return hram[address - HRAM_ADDR];
    }
    else if (address == IE_ADDR)
    {
        return mCPU.IE;
    }

    return 0xFF;
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
        int finalAddress = address - VRAM_ADDR;
        if (mPPU.VRAMBank)
        {
            finalAddress += PPU::VRAM_BANK_SIZE;
        }
        mPPU.vram[finalAddress] = value;
    }
    else if (address >= SRAM_ADDR && address < WRAM0_ADDR)
    {
        mCart.writeByte(address, value);
    }
    else if (address >= WRAM0_ADDR && address < ECHO_ADDR)
    {
        int finalAddress = address - WRAM0_ADDR;
        if (address >= WRAM1_ADDR)
        {
            finalAddress += WRAM_BANK_SIZE * (mWRAMBank - 1);
        }
        wram[finalAddress] = value;
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
        mAPU.writeNRx1(0, value);
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
        mAPU.writeNRx4(0, value);
    }
    else if (address == NR21_ADDR)
    {
        mAPU.writeNRx1(1, value);
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
        mAPU.writeNRx4(1, value);
    }
    else if (address == NR30_ADDR)
    {
        mAPU.NR30 = value;
    }
    else if (address == NR31_ADDR)
    {
        mAPU.writeNRx1(2, value);
    }
    else if (address == NR32_ADDR)
    {
        mAPU.NRx2[2] = value;
    }
    else if (address == NR33_ADDR)
    {
        mAPU.NRx3[2] = value;
    }
    else if (address == NR34_ADDR)
    {
        mAPU.writeNRx4(2, value);
    }
    else if (address == NR41_ADDR)
    {
        mAPU.writeNRx1(3, value);
    }
    else if (address == NR42_ADDR)
    {
        mAPU.NRx2[3] = value;
    }
    else if (address == NR43_ADDR)
    {
        mAPU.NRx3[3] = value;
    }
    else if (address == NR44_ADDR)
    {
        mAPU.writeNRx4(3, value);
    }
    else if (address == NR50_ADDR)
    {
        mAPU.NR50 = value;
    }
    else if (address == NR51_ADDR)
    {
        mAPU.NR51 = value;
    }
    else if (address == NR52_ADDR)
    {
        mAPU.writeNR52(value);
    }
    else if (address >= WAVE_RAM_START_ADDR && address <= WAVE_RAM_END_ADDR)
    {
        mAPU.waveRam[address - WAVE_RAM_START_ADDR] = value;
    }
    else if (address == LCDC_ADDR)
    {
        mPPU.writeLCDC(value);
    }
    else if (address == STAT_ADDR)
    {
        mPPU.writeSTAT(value);
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
        mPPU.writeLYC(value);
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
    else if (address == KEY0_ADDR)
    {
        if (!(mBootRomFinished))
        {
            mKEY0 = value & (1 << 2);
        }
    }
    else if (address == KEY1_ADDR && inCGBMode())
    {
        mCPU.armedSpeedSwitch = value & 1;
    }
    else if (address == VBK_ADDR && inCGBMode())
    {
        mPPU.VRAMBank = value & 1;
    }
    else if (address == BANK_ADDR)
    {
        if (!(mBootRomFinished) && value)
        {
            mBootRomFinished = true;
        }
    }
    else if (address == HDMA1_ADDR && inCGBMode())
    {
        mCPU.hdmaSrc &= 0xFF;
        mCPU.hdmaSrc |= (value << 8);
    }
    else if (address == HDMA2_ADDR && inCGBMode())
    {
        mCPU.hdmaSrc &= 0xFF00;
        mCPU.hdmaSrc |= (value & 0xF0);
    }
    else if (address == HDMA3_ADDR && inCGBMode())
    {
        mCPU.hdmaDest &= 0xE0FF;
        mCPU.hdmaDest |= ((value & 0x1F) << 8);
    }
    else if (address == HDMA4_ADDR && inCGBMode())
    {
        mCPU.hdmaDest &= 0xFF00;
        mCPU.hdmaDest |= (value & 0xF0);
    }
    else if (address == HDMA5_ADDR && inCGBMode())
    {
        int hdmaLength = ((value & 0x7F) + 1) << 4;
        switch (value & (1 << 7))
        {
        case 0:
            mCPU.startGeneralDMA(hdmaLength);
            break;

        case (1 << 7):
            mCPU.startHBlankDMA(hdmaLength);
            break;

        default:
            break;
        }
    }
    else if (address == BCPS_ADDR)
    {
        mLCD.BCPS = value;
    }
    else if (address == BCPD_ADDR)
    {
        int address = mLCD.BCPS & 63;
        mLCD.colorBGPaletteMemory[address] = value;
        if (mLCD.BCPS & LCD::CPS_FLAG_AUTOINC)
        {
            address = (address + 1) & 63;
            writeByte(BCPS_ADDR, (mLCD.BCPS & 0b11000000) | address);
        }
    }
    else if (address == OCPS_ADDR)
    {
        mLCD.OCPS = value;
    }
    else if (address == OCPD_ADDR)
    {
        int address = mLCD.OCPS & 63;
        mLCD.colorOBJPaletteMemory[address] = value;
        if (mLCD.OCPS & LCD::CPS_FLAG_AUTOINC)
        {
            address = (address + 1) & 63;
            writeByte(OCPS_ADDR, (mLCD.OCPS & 0b11000000) | address);
        }
    }
    else if (address == SVBK_ADDR && inCGBMode())
    {
        mWRAMBank = value & 0b111;
        if (mWRAMBank == 0)
        {
            mWRAMBank = 1;
        }
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

const LCD::Color *Gameboy::getPixels() const
{
    return mLCD.pixels;
}

void Gameboy::computeAudioSamples()
{
    mAPU.computeAudioSamples();
}

float Gameboy::getLeftAudioSample() const
{
    return mAPU.leftAudioSample;
}

float Gameboy::getRightAudioSample() const
{
    return mAPU.rightAudioSample;
}

void Gameboy::doTCycle()
{
    int cpuCycleDiv = CPU_CYCLE_DIV / (mCPU.isDoubleSpeed() ? 2 : 1);
    if (!(tCycleCounter % cpuCycleDiv))
    {
        mCPU.doMCycle();
    }
    mPPU.doCycle();
    if (!(tCycleCounter % APU_CYCLE_DIV))
    {
        mAPU.doCycle();
    }
    tCycleCounter++;
}

void Gameboy::onKeyPress(int key)
{
    if ((key < Joypad::BUTTONS_INDEX && mJoypad.selectDPad && !mJoypad.keys[key] && !mJoypad.keys[key + Joypad::BUTTONS_INDEX])
        || (key >= Joypad::BUTTONS_INDEX && mJoypad.selectButtons && !mJoypad.keys[key] && !mJoypad.keys[key - Joypad::BUTTONS_INDEX]))
    {
        requestInterrupt(CPU::InterruptSource::Joypad);
    }
    mJoypad.keys[key] = true;
}

void Gameboy::onKeyRelease(int key)
{
    mJoypad.keys[key] = false;
}

Cartridge &Gameboy::getCart()
{
    return mCart;
}

void Gameboy::setDrawCallback(void (*drawCallback)(void *), void *userdata)
{
    mPPU.setDrawCallback(drawCallback, userdata);
}

void Gameboy::setBootRom(std::istream &dataStream)
{
    switch (mSystemType)
    {
    case DMG:
        dataStream.read(reinterpret_cast<char *>(bootRom), dmgBootRomSize);
        break;

    case CGB:
        dataStream.read(reinterpret_cast<char *>(bootRom), cgbBootRomSize);
        break;

    default:
        break;
    }
    mBootRomFinished = false;
    mCPU.loadBootRom();
}

void Gameboy::simulateBootRom()
{
    if (mSystemType == SystemType::CGB)
    {
        uint8_t cgbFlag = mCart.getHeader().cgbFlag;
        if (cgbFlag & (1 << 7))
        {
            mKEY0 = cgbFlag;
        }
        else
        {
            mKEY0 = 0x04;
        }

        if (inCGBMode())
        {
            std::memset(mLCD.colorBGPaletteMemory, 0, 64);
        }
        else
        {
            // Fallback monochrome palette for DMG mode
            std::memcpy(mLCD.colorBGPaletteMemory, DMG_MODE_FALLBACK_PALETTE, 8);
            std::memcpy(mLCD.colorOBJPaletteMemory, DMG_MODE_FALLBACK_PALETTE, 8);
            std::memcpy(mLCD.colorOBJPaletteMemory + 8, DMG_MODE_FALLBACK_PALETTE, 8);
        }
    }
    mCPU.simulateBootRom();
}

Gameboy::SystemType Gameboy::getSystemType()
{
    return mSystemType;
}

bool Gameboy::inDMGMode()
{
    return mKEY0 & 0x04;
}

bool Gameboy::inCGBMode()
{
    return mSystemType == CGB && !inDMGMode();
}

bool Gameboy::inHBlank()
{
    return mPPU.getMode() == 0;
}
