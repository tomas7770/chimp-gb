#include "MBC3.h"

#include <ctime>

uint8_t MBC3::readByte(std::vector<uint8_t> &romData, uint16_t address)
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
        if (mRAM_RTC_Select <= 0x03)
        {
            bigAddress += RAM_BANK_SIZE * mRAM_RTC_Select;
            return mRAM[bigAddress - SRAM_ADDR];
        }
        else if (mHasClock)
        {
            switch (mRAM_RTC_Select)
            {
            case 0x08:
                return mRTC.latchedTimeSeconds;
            case 0x09:
                return mRTC.latchedTimeMinutes;
            case 0x0A:
                return mRTC.latchedTimeHours;
            case 0x0B:
                return mRTC.latchedTimeDays;
            case 0x0C:
                return mRTC.latchedTimeDaysHigh;

            default:
                break;
            }
        }
    }
    return 0xFF;
}

void MBC3::writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value)
{
    if (address <= RAM_ENABLE_END)
    {
        mRAMEnabled = (value & 0b1111) == 0xA;
    }
    else if (address >= ROM_BANK_SELECT_START && address <= ROM_BANK_SELECT_END)
    {
        mROMBank = value & 0b1111111;
        if (mROMBank == 0)
        {
            mROMBank = 1;
        }
    }
    else if (address >= RAM_BANK_SELECT_START && address <= RAM_BANK_SELECT_END)
    {
        mRAM_RTC_Select = value & 0xF;
    }
    else if (address >= LATCH_CLOCK_START && address <= LATCH_CLOCK_END && mHasClock)
    {
        if (mLatchClockReg == 0x00 && value == 0x01)
        {
            updateRTC();
            mRTC.latchedTimeSeconds = mRTC.timeSeconds;
            mRTC.latchedTimeMinutes = mRTC.timeMinutes;
            mRTC.latchedTimeHours = mRTC.timeHours;
            mRTC.latchedTimeDays = mRTC.timeDays;
            mRTC.latchedTimeDaysHigh = mRTC.timeDaysHigh;
        }
        mLatchClockReg = value;
    }
    else if (address >= SRAM_ADDR && mRAMEnabled)
    {
        if (mRAM_RTC_Select <= 0x03)
        {
            int bigAddress = address;
            bigAddress += RAM_BANK_SIZE * mRAM_RTC_Select;
            mRAM[bigAddress - SRAM_ADDR] = value;
        }
        else if (mHasClock)
        {
            updateRTC();
            switch (mRAM_RTC_Select)
            {
            case 0x08:
                mRTC.timeSeconds = value % 60;
                mRTC.latchedTimeSeconds = value % 60;
                break;
            case 0x09:
                mRTC.timeMinutes = value % 60;
                mRTC.latchedTimeMinutes = value % 60;
                break;
            case 0x0A:
                mRTC.timeHours = value % 24;
                mRTC.latchedTimeHours = value % 24;
                break;
            case 0x0B:
                mRTC.timeDays = value;
                mRTC.latchedTimeDays = value;
                break;
            case 0x0C:
                mRTC.timeDaysHigh = value;
                mRTC.latchedTimeDaysHigh = value;
                break;

            default:
                break;
            }
        }
    }
}

void MBC3::updateRTC()
{
    std::time_t currentTime = std::time(nullptr);
    if (!(mRTC.timeDaysHigh & CLOCK_HALT_BITMASK))
    {
        std::time_t timePassed = currentTime - mRTC.timestamp;
        std::time_t secondsPassed = timePassed % 60;
        std::time_t minutesPassed = (timePassed / 60) % 60;
        std::time_t hoursPassed = (timePassed / (60 * 60)) % 24;
        std::time_t daysPassed = timePassed / (24 * 60 * 60);
        int fullTimeDays = ((mRTC.timeDaysHigh & 1) << 8) | mRTC.timeDays;

        mRTC.timeSeconds += secondsPassed;
        if (mRTC.timeSeconds >= 60)
        {
            mRTC.timeSeconds -= 60;
            mRTC.timeMinutes++;
        }
        mRTC.timeMinutes += minutesPassed;
        if (mRTC.timeMinutes >= 60)
        {
            mRTC.timeMinutes -= 60;
            mRTC.timeHours++;
        }
        mRTC.timeHours += hoursPassed;
        if (mRTC.timeHours >= 24)
        {
            mRTC.timeHours -= 24;
            fullTimeDays++;
        }

        fullTimeDays += daysPassed;
        if (fullTimeDays >= 0x200)
        {
            fullTimeDays %= 0x200;
            mRTC.timeDaysHigh |= CLOCK_CARRY_BITMASK;
        }
        mRTC.timeDays = fullTimeDays & 0xFF;
        mRTC.timeDaysHigh = (fullTimeDays & (1 << 8)) ? (mRTC.timeDaysHigh | 1) : (mRTC.timeDaysHigh & ~1);
    }
    mRTC.timestamp = currentTime;
}
