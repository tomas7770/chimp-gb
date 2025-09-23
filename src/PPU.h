#pragma once

#include <cstdint>
#include <vector>
#include "LCD.h"

class Gameboy;

class PPU
{
public:
    PPU(Gameboy *gameboy, LCD *lcd) : mGameboy(gameboy), mLCD(lcd) {}

    static constexpr int vramSize = 16384;
    uint8_t vram[vramSize];
    static constexpr int oamSize = 0xFE9F - 0xFE00 + 1;
    uint8_t oam[oamSize];

    int VRAMBank = 0;

    int getMode() const;
    void writeLCDC(uint8_t value);
    void writeSTAT(uint8_t value);
    void writeLYC(uint8_t value);
    void setDrawCallback(void (*drawCallback)(void *), void *userdata);

    void doDot();

    static constexpr uint16_t VRAM_BANK_SIZE = (1 << 13);

private:
    Gameboy *mGameboy;
    LCD *mLCD;

    bool mEnabled = true;
    int mMode = 2;
    int mCurrentDot = 0; // current dot in current line, not total
    bool mIncrementedWindowLine = false;
    int mStatInterruptLine = 0;

    std::vector<int> spritesInScanline; // list of sprites to draw, stored as an offset from OAM start

    void (*drawCallback)(void *userdata) = nullptr;
    void *mDrawCallbackUserdata = nullptr;

    void setMode(int mode);
    void updateStatInterruptLine();

    uint8_t getBGTileAtScreenPixel(int x, int y, bool isWindow);
    int getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY, bool drawingObj,
                       bool xFlip = false, bool yFlip = false);
    int getBGPixelOnScreen(int x, int y);
    int getWindowPixel(int x, int y);
    LCD::Color getPaletteColor(uint8_t palette, int colorId);
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

    static constexpr uint16_t TILE_BLOCK_2_OFFSET = 0x9000 - 0x8000;

    static constexpr uint16_t TILE_MAP_0_ADDR = 0x9800 - 0x8000;
    static constexpr uint16_t TILE_MAP_1_ADDR = 0x9C00 - 0x8000;

    static constexpr int SPRITE_BYTES = 4;
    static constexpr int MAX_SPRITES_PER_LINE = 10;

    static constexpr uint8_t OBJ_FLAG_PRIORITY = (1 << 7);
    static constexpr uint8_t OBJ_FLAG_Y_FLIP = (1 << 6);
    static constexpr uint8_t OBJ_FLAG_X_FLIP = (1 << 5);
    static constexpr uint8_t OBJ_FLAG_DMG_PAL = (1 << 4);
};
