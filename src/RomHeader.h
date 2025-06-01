#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct RomHeader
{
    enum CartridgeType
    {
        ROM_ONLY,
        CART_TYPE_COUNT
    };

    std::string title;

    RomHeader(const std::vector<uint8_t> &romData);
    RomHeader() = default;
};
