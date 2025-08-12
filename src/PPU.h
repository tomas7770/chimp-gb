#pragma once

#include <cstdint>
#include <vector>
#include "LCD.h"

class Gameboy;

class PPU
{
public:
    PPU(Gameboy *gameboy, LCD *lcd) : mGameboy(gameboy), mLCD(lcd) {}

    static constexpr int vramSize = 8192;
    uint8_t vram[vramSize];
    static constexpr int oamSize = 0xFE9F - 0xFE00 + 1;
    uint8_t oam[oamSize];

    int getMode() const;

    void doDot();

private:
    Gameboy *mGameboy;
    LCD *mLCD;

    int mMode = 2;
    int mCurrentDot = 0; // current dot in current line, not total

    std::vector<int> spritesInScanline; // list of sprites to draw, stored as an offset from OAM start

    uint8_t getBGTileAtScreenPixel(int x, int y);
    int getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY);
    int getBGPixelOnScreen(int x, int y);
    LCD::Color getScreenPixel(int x, int y);

    static constexpr int MODE_2_DOTS = 80;
    static constexpr int MODE_3_DOTS = 172; // inaccurate, this is variable
    static constexpr int MODE_3_DUMMY_DOTS = 12;
    static constexpr int MODE_0_DOTS = 376 - MODE_3_DOTS; // inaccurate, this is variable
    static constexpr int MODE_1_DOTS = 456;
    static constexpr int DOTS_PER_LINE = 456;
    static constexpr int VBLANK_LINES = 10;

    static constexpr int TILE_LENGTH = 8;
    static constexpr int TILE_MAP_LENGTH = 32;
    static constexpr int TILE_BYTES = 16;

    static constexpr uint16_t TILE_MAP_0_ADDR = 0x9800 - 0x8000;

    static constexpr int SPRITE_BYTES = 4;
    static constexpr int MAX_SPRITES_PER_LINE = 10;
};
