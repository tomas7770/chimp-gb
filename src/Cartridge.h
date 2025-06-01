#pragma once

#include <vector>
#include <istream>

class Cartridge
{
public:
    Cartridge(std::istream &dataStream, std::streamsize size);
    const std::vector<std::byte> &getData() const;

private:
    std::vector<std::byte> mData;
};
