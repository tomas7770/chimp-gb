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
    uint8_t STAT = 0x85;
    uint8_t SCY = 0;
    uint8_t SCX = 0;
    uint8_t LY = 0;
    uint8_t LYC = 0;
    uint8_t BGP = 0xFC;
    uint8_t OBP0;
    uint8_t OBP1;
    uint8_t WY = 0;
    uint8_t WX = 0;
    uint8_t windowLineCounter = 0;

    static constexpr uint8_t LCDC_FLAG_LCD_PPU_ENABLE = (1 << 7);
    static constexpr uint8_t LCDC_FLAG_WINDOW_TILE_MAP = (1 << 6);
    static constexpr uint8_t LCDC_FLAG_WINDOW_ENABLE = (1 << 5);
    static constexpr uint8_t LCDC_FLAG_BG_WINDOW_TILE_DATA = (1 << 4);
    static constexpr uint8_t LCDC_FLAG_BG_TILE_MAP = (1 << 3);
    static constexpr uint8_t LCDC_FLAG_OBJ_SIZE = (1 << 2);
    static constexpr uint8_t LCDC_FLAG_OBJ_ENABLE = (1 << 1);
    static constexpr uint8_t LCDC_FLAG_BG_WINDOW_ENABLE = (1 << 0);

    static constexpr uint8_t STAT_LYC_INT_BITMASK = (1 << 6);
    static constexpr uint8_t STAT_MODE_2_INT_BITMASK = (1 << 5);
    static constexpr uint8_t STAT_MODE_1_INT_BITMASK = (1 << 4);
    static constexpr uint8_t STAT_MODE_0_INT_BITMASK = (1 << 3);
    static constexpr uint8_t STAT_LYC_LY_BITMASK = (1 << 2);
    static constexpr uint8_t STAT_MODE_BITMASK = (1 << 1) | (1 << 0);
};
