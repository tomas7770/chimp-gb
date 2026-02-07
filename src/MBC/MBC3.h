#pragma once

#include "MBC.h"

// Note: The RTC implementation syncs to the host's time. This means there should be no drift when emulating
// at speeds other than 100%. On the other hand, it will desync relative to the emulated system's time, which
// is inaccurate to real hardware.
class MBC3 : public MBC
{
public:
    MBC3(bool hasBattery, bool hasClock = false) : MBC(hasBattery), mHasClock(hasClock) {}

    uint8_t readByte(std::vector<uint8_t> &romData, uint16_t address) override;
    void writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value) override;

    RTC *getRTC() override { return &mRTC; }
    bool hasClock() override { return mHasClock; }

private:
    int mROMBank = 1;
    int mRAM_RTC_Select = 0;

    bool mHasClock;
    RTC mRTC;
    int mLatchClockReg;

    bool mRAMEnabled = false;

    void updateRTC();

    static constexpr uint16_t RAM_ENABLE_END = 0x1FFF;

    static constexpr uint16_t ROM_BANK_SELECT_START = 0x2000;
    static constexpr uint16_t ROM_BANK_SELECT_END = 0x3FFF;

    static constexpr uint16_t RAM_BANK_SELECT_START = 0x4000;
    static constexpr uint16_t RAM_BANK_SELECT_END = 0x5FFF;

    static constexpr uint16_t LATCH_CLOCK_START = 0x6000;
    static constexpr uint16_t LATCH_CLOCK_END = 0x7FFF;

    static constexpr uint8_t CLOCK_CARRY_BITMASK = (1 << 7);
    static constexpr uint8_t CLOCK_HALT_BITMASK = (1 << 6);
};
