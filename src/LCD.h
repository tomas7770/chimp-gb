#pragma once

#include <cstdint>

struct LCD
{
    static constexpr int SCREEN_W = 160;
    static constexpr int SCREEN_H = 144;

    enum Color
    {
        White,
        LightGray,
        DarkGray,
        Black
    };

    Color pixels[SCREEN_W * SCREEN_H] = {Color::White};

    uint8_t LCDC = 0x91;
    uint8_t LY = 0;
};
