#pragma once

#include "RomHeader.h"
#include "MBC/MBC.h"

#include <vector>
#include <istream>
#include <cstdint>
#include <memory>

class Cartridge
{
public:
    Cartridge(std::istream &dataStream, std::streamsize size);
    const RomHeader &getHeader() const;

    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);

    bool hasBattery() const;

private:
    std::vector<uint8_t> mData;
    RomHeader mHeader;
    std::shared_ptr<MBC> mMBC;
};
