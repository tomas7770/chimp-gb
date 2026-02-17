#pragma once

#include "MBC.h"

class MBC5 : public MBC
{
public:
    MBC5(bool hasBattery) : MBC(hasBattery) {}

    uint8_t readByte(std::vector<uint8_t> &romData, uint16_t address) override;
    void writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value) override;

private:
    int mROMBank = 1;
    int mRAMBank = 0;
    bool mRAMEnabled = false;
    bool mRAMDirty = false;

    static constexpr uint16_t RAM_ENABLE_END = 0x1FFF;

    static constexpr uint16_t ROM_BANK_SELECT_LOW_START = 0x2000;
    static constexpr uint16_t ROM_BANK_SELECT_LOW_END = 0x2FFF;

    static constexpr uint16_t ROM_BANK_SELECT_HIGH_START = 0x3000;
    static constexpr uint16_t ROM_BANK_SELECT_HIGH_END = 0x3FFF;

    static constexpr uint16_t RAM_BANK_SELECT_START = 0x4000;
    static constexpr uint16_t RAM_BANK_SELECT_END = 0x5FFF;
};
