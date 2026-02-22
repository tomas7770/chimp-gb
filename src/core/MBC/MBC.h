#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include "SaveState.h"

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

        // rtcBytes must be 48 bytes long
        void serialize(uint8_t *rtcBytes) const
        {
            rtcBytes[0] = timeSeconds;
            rtcBytes[4] = timeMinutes;
            rtcBytes[8] = timeHours;
            rtcBytes[12] = timeDays;
            rtcBytes[16] = timeDaysHigh;
            rtcBytes[20] = latchedTimeSeconds;
            rtcBytes[24] = latchedTimeMinutes;
            rtcBytes[28] = latchedTimeHours;
            rtcBytes[32] = latchedTimeDays;
            rtcBytes[36] = latchedTimeDaysHigh;
            memcpy(rtcBytes + 40, &timestamp, 8);
        }
    };
    virtual RTC *getRTC() { return nullptr; }
    virtual bool hasClock() { return false; }

    void setSaveCallback(void (*saveCallback)(void *), void *userdata)
    {
        this->saveCallback = saveCallback;
        mSaveCallbackUserdata = userdata;
    }

    virtual void saveStateMBCBlock(SaveState &state) {}
    void saveState(SaveState &state)
    {
        memcpy(state.sram, mRAM, 131072);

        saveStateMBCBlock(state);

        auto rtc = getRTC();
        if (rtc != nullptr)
        {
            rtc->serialize(state.rtcBytes);
        }
    }

    static constexpr uint16_t SWITCHABLE_BANK_ADDR = 0x4000;
    static constexpr uint16_t ROM_END = 0x7FFF;
    static constexpr uint16_t ROM_BANK_SIZE = (1 << 14);

    static constexpr uint16_t SRAM_ADDR = 0xA000;
    static constexpr uint16_t RAM_BANK_SIZE = (1 << 13);

protected:
    uint8_t mRAM[131072];
    void (*saveCallback)(void *userdata) = nullptr;
    void *mSaveCallbackUserdata = nullptr;

private:
    bool mHasBattery;
};
