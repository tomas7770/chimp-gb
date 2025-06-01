#include "Gameboy.h"

#include <iostream>

void Gameboy::printDataSize() const
{
    std::cout << mCart.getData().size() << std::endl;
}
