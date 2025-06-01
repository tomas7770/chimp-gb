#include "Cartridge.h"

Cartridge::Cartridge(std::istream &dataStream, std::streamsize size)
{
    if (size < 0x014F)
    {
        throw std::runtime_error("Provided ROM is not valid: Smaller than header size");
    }
    mData.resize(size);
    dataStream.read(reinterpret_cast<char *>(mData.data()), size);

    mHeader = RomHeader(mData);
}

const std::vector<std::uint8_t> &Cartridge::getData() const
{
    return mData;
}

const RomHeader &Cartridge::getHeader() const
{
    return mHeader;
}
