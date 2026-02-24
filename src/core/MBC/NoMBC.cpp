#include "NoMBC.h"

uint8_t NoMBC::readByte(std::vector<uint8_t> &romData, uint16_t address)
{
    if (address > MBC::ROM_END)
    {
        return 0xFF;
    }
    return romData.at(address);
}
