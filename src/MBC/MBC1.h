#pragma once

#include "MBC.h"

class MBC1 : public MBC
{
    // TODO handle 1MB+ ROMs differently
public:
    MBC1(bool hasBattery) : MBC(hasBattery) {}

    uint8_t readByte(std::vector<uint8_t> &romData, uint16_t address) override;
    void writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value) override;

private:
    int mROMBank = 1;
    int mRAMBank = 0;
    bool mRAMEnabled = false;
    bool mRAMDirty = false;

    static constexpr uint16_t RAM_ENABLE_END = 0x1FFF;

    static constexpr uint16_t ROM_BANK_SELECT_START = 0x2000;
    static constexpr uint16_t ROM_BANK_SELECT_END = 0x3FFF;

    static constexpr uint16_t RAM_BANK_SELECT_START = 0x4000;
    static constexpr uint16_t RAM_BANK_SELECT_END = 0x5FFF;
};
