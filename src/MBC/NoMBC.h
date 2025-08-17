#pragma once

#include "MBC.h"

class NoMBC : public MBC
{
public:
    uint8_t readByte(std::vector<uint8_t> &romData, uint16_t address) override;
    void writeByte(std::vector<uint8_t> &romData, uint16_t address, uint8_t value) override {};
};
