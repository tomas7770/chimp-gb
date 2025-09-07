#pragma once

#include <cstdint>
#include <vector>

class MBC
{
public:
    MBC() : mHasBattery(false) {}
    MBC(bool hasBattery) : mHasBattery(hasBattery) {}

    virtual uint8_t readByte(std::vector<uint8_t> &romData, uint16_t address) = 0;
    virtual void writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value) {}
    bool hasBattery() const { return mHasBattery; }
    uint8_t *getRAM() { return mRAM; }

    struct RTC
    {
        uint8_t timeSeconds;
        uint8_t timeMinutes;
        uint8_t timeHours;
        uint8_t timeDays;
        uint8_t timeDaysHigh;
        uint8_t latchedTimeSeconds;
        uint8_t latchedTimeMinutes;
        uint8_t latchedTimeHours;
        uint8_t latchedTimeDays;
        uint8_t latchedTimeDaysHigh;
        uint64_t timestamp;
    };
    virtual RTC *getRTC() { return nullptr; }
    virtual bool hasClock() { return false; }

    static constexpr uint16_t SWITCHABLE_BANK_ADDR = 0x4000;
    static constexpr uint16_t ROM_END = 0x7FFF;
    static constexpr uint16_t ROM_BANK_SIZE = (1 << 14);

    static constexpr uint16_t SRAM_ADDR = 0xA000;
    static constexpr uint16_t RAM_BANK_SIZE = (1 << 13);

protected:
    uint8_t mRAM[131072];

private:
    bool mHasBattery;
};
