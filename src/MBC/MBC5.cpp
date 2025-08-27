#include "MBC5.h"

uint8_t MBC5::readByte(std::vector<uint8_t> &romData, uint16_t address)
{
    int bigAddress = address; // need an int greater than 16-bits for ROMs larger than 64 KiB
    if (bigAddress <= MBC::ROM_END)
    {
        if (bigAddress >= MBC::SWITCHABLE_BANK_ADDR)
        {
            bigAddress += ROM_BANK_SIZE * (mROMBank - 1);
        }
        return romData.at(bigAddress);
    }
    else if (bigAddress >= MBC::SRAM_ADDR)
    {
        bigAddress += RAM_BANK_SIZE * mRAMBank;
        return mRAM[bigAddress - SRAM_ADDR];
    }
    return 0;
}

void MBC5::writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value)
{
    if (address >= ROM_BANK_SELECT_LOW_START && address <= ROM_BANK_SELECT_LOW_END)
    {
        mROMBank &= ~0xFF;
        mROMBank |= value;
    }
    else if (address >= ROM_BANK_SELECT_HIGH_START && address <= ROM_BANK_SELECT_HIGH_END)
    {
        mROMBank &= ~(1 << 8);
        mROMBank |= (value & 1) << 8;
    }
    else if (address >= RAM_BANK_SELECT_START && address <= RAM_BANK_SELECT_END)
    {
        mRAMBank = value % 16;
    }
    else if (address >= SRAM_ADDR)
    {
        int bigAddress = address;
        bigAddress += RAM_BANK_SIZE * mRAMBank;
        mRAM[bigAddress - SRAM_ADDR] = value;
    }
}
