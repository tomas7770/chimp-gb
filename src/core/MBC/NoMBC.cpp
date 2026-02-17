#include "NoMBC.h"

uint8_t NoMBC::readByte(std::vector<uint8_t> &romData, uint16_t address)
{
    return romData.at(address);
}
