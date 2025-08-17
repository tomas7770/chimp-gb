#include "MBC1.h"

uint8_t MBC1::readByte(std::vector<uint8_t> &romData, uint16_t address)
{
    int romAddress = address; // need an int greater than 16-bits for ROMs larger than 64 KiB
    if (romAddress >= MBC::SWITCHABLE_BANK_ADDR)
    {
        romAddress += ROM_BANK_SIZE * (mROMBank - 1);
    }
    return romData.at(romAddress);
}

void MBC1::writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value)
{
    if (address >= ROM_BANK_SELECT_START && address <= ROM_BANK_SELECT_END)
    {
        mROMBank = value & 0b11111;
        if (mROMBank == 0)
        {
            mROMBank = 1;
        }
    }
}
