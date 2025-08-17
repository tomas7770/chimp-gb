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
        CART_TYPE_COUNT
    };

    std::string title;
    CartridgeType cartType;

    RomHeader(const std::vector<uint8_t> &romData);
    RomHeader() = default;
};
