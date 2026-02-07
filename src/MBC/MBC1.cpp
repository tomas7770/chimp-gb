#include "MBC1.h"

uint8_t MBC1::readByte(std::vector<uint8_t> &romData, uint16_t address)
{
    int bigAddress = address; // need an int greater than 16-bits for ROMs larger than 64 KiB
    if (bigAddress <= MBC::ROM_END)
    {
        if (bigAddress >= MBC::SWITCHABLE_BANK_ADDR)
        {
            bigAddress += ROM_BANK_SIZE * (mROMBank - 1);
        }
        return romData.at(bigAddress % romData.size());
    }
    else if (bigAddress >= MBC::SRAM_ADDR && mRAMEnabled)
    {
        bigAddress += RAM_BANK_SIZE * mRAMBank;
        return mRAM[bigAddress - SRAM_ADDR];
    }
    return 0xFF;
}

void MBC1::writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value)
{
    if (address <= RAM_ENABLE_END)
    {
        mRAMEnabled = (value & 0b1111) == 0xA;
    }
    else if (address >= ROM_BANK_SELECT_START && address <= ROM_BANK_SELECT_END)
    {
        mROMBank = value & 0b11111;
        if (mROMBank == 0)
        {
            mROMBank = 1;
        }
    }
    else if (address >= RAM_BANK_SELECT_START && address <= RAM_BANK_SELECT_END)
    {
        mRAMBank = value % 4;
    }
    else if (address >= SRAM_ADDR && mRAMEnabled)
    {
        int bigAddress = address;
        bigAddress += RAM_BANK_SIZE * mRAMBank;
        mRAM[bigAddress - SRAM_ADDR] = value;
    }
}
