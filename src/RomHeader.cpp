#include "RomHeader.h"

#include <stdexcept>
#include <cstring>

RomHeader::RomHeader(const std::vector<uint8_t> &romData)
{
    if (romData.size() < 0x014F)
    {
        throw std::logic_error("ROM data smaller than header size");
    }

    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++)
    {
        checksum = checksum - romData[address] - 1;
    }
    if (checksum != romData[0x014D])
    {
        throw std::runtime_error("Provided ROM is not valid: Checksum mismatch");
    }

    if (romData[0x0147] >= CART_TYPE_COUNT)
    {
        // throw std::runtime_error("Provided ROM is not yet supported");
    }

    char titleChars[16];
    std::memcpy(titleChars, romData.data() + 0x0134, 16);
    title = std::string(titleChars);
}
