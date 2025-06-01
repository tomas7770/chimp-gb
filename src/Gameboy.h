#pragma once

#include "Cartridge.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart) : mCart(std::move(cart)) {}
    void printDataSize() const;

private:
    Cartridge mCart;
};
