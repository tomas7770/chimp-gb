#include "Cartridge.h"

#include "MBC/NoMBC.h"
#include "MBC/MBC1.h"
#include "MBC/MBC3.h"

Cartridge::Cartridge(std::istream &dataStream, std::streamsize size)
{
    if (size < 0x014F)
    {
        throw std::runtime_error("Provided ROM is not valid: Smaller than header size");
    }
    mData.resize(size);
    dataStream.read(reinterpret_cast<char *>(mData.data()), size);

    mHeader = RomHeader(mData);

    switch (mHeader.cartType)
    {
    default:
    case RomHeader::CartridgeType::ROM_ONLY:
        mMBC = std::make_shared<NoMBC>();
        break;
    case RomHeader::CartridgeType::MBC1:
    case RomHeader::CartridgeType::MBC1_RAM:
        mMBC = std::make_shared<MBC1>(false);
        break;
    case RomHeader::CartridgeType::MBC1_RAM_BATTERY:
        mMBC = std::make_shared<MBC1>(true);
        break;
    case RomHeader::CartridgeType::MBC3:
    case RomHeader::CartridgeType::MBC3_RAM:
        mMBC = std::make_shared<MBC3>(false);
        break;
    case RomHeader::CartridgeType::MBC3_RAM_BATTERY:
        mMBC = std::make_shared<MBC3>(true);
        break;
    }
}

const RomHeader &Cartridge::getHeader() const
{
    return mHeader;
}

uint8_t Cartridge::readByte(uint16_t address)
{
    return mMBC->readByte(mData, address);
}

void Cartridge::writeByte(uint16_t address, uint8_t value)
{
    mMBC->writeByte(mData, address, value);
}

bool Cartridge::hasBattery() const
{
    return mMBC->hasBattery();
}
