#pragma once

#include <cstdint>

struct LCD
{
    static constexpr int SCREEN_W = 160;
    static constexpr int SCREEN_H = 144;

    // The values of this enum match exactly the DMG color indices. Do not change the order.
    enum Color
    {
        White,
        LightGray,
        DarkGray,
        Black
    };

    Color pixels[SCREEN_W * SCREEN_H] = {Color::White};

    uint8_t LCDC = 0x91;
    uint8_t SCY = 0;
    uint8_t SCX = 0;
    uint8_t LY = 0;
    uint8_t BGP = 0xFC;
    uint8_t OBP0;
    uint8_t OBP1;
};
