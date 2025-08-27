#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct RomHeader
{
    enum CartridgeType
    {
        ROM_ONLY,
        MBC1,
        MBC1_RAM,
        MBC1_RAM_BATTERY,
        MBC3 = 0x11,
        MBC3_RAM = 0x12,
        MBC3_RAM_BATTERY = 0x13,
        MBC5 = 0x19,
        MBC5_RAM = 0x1A,
        MBC5_RAM_BATTERY = 0x1B,
    };

    std::string title;
    CartridgeType cartType;

    RomHeader(const std::vector<uint8_t> &romData);
    RomHeader() = default;
};
