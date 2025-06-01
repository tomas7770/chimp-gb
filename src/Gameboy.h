#pragma once

#include "Cartridge.h"
#include "CPU.h"

class Gameboy
{
public:
    Gameboy(const Cartridge &cart) : mCart(std::move(cart)) {}
    void printDataSize() const;

    static constexpr int wramSize = 8192;
    uint8_t wram[wramSize];

private:
    Cartridge mCart;
    CPU mCPU;
};
