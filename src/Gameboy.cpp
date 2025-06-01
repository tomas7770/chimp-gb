#include "Gameboy.h"

#include <iostream>

void Gameboy::printDataSize() const
{
    std::cout << mCart.getHeader().title << std::endl;
}
