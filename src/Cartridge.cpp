#include "Cartridge.h"

Cartridge::Cartridge(std::istream &dataStream, std::streamsize size)
{
    mData.resize(size);
    dataStream.read(reinterpret_cast<char *>(mData.data()), size);
}

const std::vector<std::byte> &Cartridge::getData() const
{
    return mData;
}
