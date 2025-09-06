#include "RomHeader.h"

#include <stdexcept>
#include <cstring>

RomHeader::RomHeader(const std::vector<uint8_t> &romData)
{
    if (romData.size() < 0x014F)
    {
        throw std::logic_error("ROM data smaller than header size");
    }

    checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++)
    {
        checksum = checksum - romData[address] - 1;
    }
    if (checksum != romData[0x014D])
    {
        throw std::runtime_error("Provided ROM is not valid: Checksum mismatch");
    }

    switch (romData[MBC_BYTE])
    {
    case ROM_ONLY:
    case MBC1:
    case MBC1_RAM:
    case MBC1_RAM_BATTERY:
    case MBC3:
    case MBC3_RAM:
    case MBC3_RAM_BATTERY:
    case MBC5:
    case MBC5_RAM:
    case MBC5_RAM_BATTERY:
        break;

    default:
        throw std::runtime_error("Provided ROM is not yet supported");
        break;
    }

    char titleChars[16];
    std::memcpy(titleChars, romData.data() + 0x0134, 16);
    title = std::string(titleChars);

    cartType = static_cast<CartridgeType>(romData[MBC_BYTE]);

    int ramSizeIndex = romData[RAM_SIZE_BYTE];
    if (ramSizeIndex > 5)
    {
        ramSize = RAM_SIZES[4]; // largest
    }
    else
    {
        ramSize = RAM_SIZES[ramSizeIndex];
    }
}
