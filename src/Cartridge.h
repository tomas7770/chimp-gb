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
    const uint8_t *getSRAM() const;
    void loadSRAM(std::istream &dataStream, std::streamsize size);

    bool hasClock() const;
    const MBC::RTC *getRTC() const;
    void loadRTC(MBC::RTC &rtcData);

private:
    std::vector<uint8_t> mData;
    RomHeader mHeader;
    std::shared_ptr<MBC> mMBC;
};
