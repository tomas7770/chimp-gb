#pragma once

#include "RomHeader.h"

#include <vector>
#include <istream>
#include <cstdint>

class Cartridge
{
public:
    Cartridge(std::istream &dataStream, std::streamsize size);
    const std::vector<uint8_t> &getData() const;
    const RomHeader &getHeader() const;

private:
    std::vector<uint8_t> mData;
    RomHeader mHeader;
};
