#pragma once

#include <cstdint>
#include <vector>

class MBC
{
public:
    virtual uint8_t readByte(std::vector<uint8_t> &romData, uint16_t address) = 0;
    virtual void writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value) = 0;

    static constexpr uint16_t SWITCHABLE_BANK_ADDR = 0x4000;
    static constexpr uint16_t ROM_BANK_SIZE = (1 << 14);
};
