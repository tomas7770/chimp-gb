#include "PPU.h"

#include "Gameboy.h"
#include "LCD.h"
#include "CPU.h"

int PPU::getMode() const
{
    return mMode;
}

void PPU::doDot()
{
    if (mMode == 3 && mCurrentDot >= MODE_2_DOTS + MODE_3_DUMMY_DOTS && mCurrentDot < MODE_2_DOTS + MODE_3_DOTS)
    {
        // Draw pixel
        int pixelX = mCurrentDot - MODE_2_DOTS - MODE_3_DUMMY_DOTS;
        int pixelY = mLCD->LY;
        int pixelCoord = pixelY * LCD::SCREEN_W + pixelX;
        mLCD->pixels[pixelCoord] = getScreenPixel(pixelX, pixelY);
    }

    mCurrentDot++;

    if (mCurrentDot >= DOTS_PER_LINE)
    {
        // Next line
        mCurrentDot = 0;
        mLCD->LY++;
        mLCD->LY %= LCD::SCREEN_H + VBLANK_LINES;
        if (mLCD->LY == LCD::SCREEN_H)
        {
            mMode = 1;
            mGameboy->requestInterrupt(CPU::InterruptSource::VBlank);
        }
        else if (mLCD->LY < LCD::SCREEN_H)
        {
            mMode = 2;
        }
    }
    else if (mLCD->LY < LCD::SCREEN_H)
    {
        if (mCurrentDot >= MODE_2_DOTS + MODE_3_DOTS)
        {
            mMode = 0;
        }
        else if (mCurrentDot >= MODE_2_DOTS)
        {
            mMode = 3;
        }
    }
}

uint8_t PPU::getBGTileAtScreenPixel(int x, int y)
{
    return vram[TILE_MAP_0_ADDR + (x / TILE_LENGTH) + TILE_MAP_LENGTH * (y / TILE_LENGTH)];
}

LCD::Color PPU::getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY)
{
    // TODO PALETTES AND ADDRESSING MODES
    int tileOffset = tileId * TILE_BYTES;
    // Each tile occupies 16 bytes, where each line is represented by 2 bytes.
    int lineOffset = tilePixelY * 2;
    // The first byte specifies the LSB of the color ID of each pixel, and the second byte specifies the MSB.
    // Bit 7 represents the leftmost pixel, and bit 0 the rightmost.
    int lsb = (vram[tileOffset + lineOffset] >> (7 - tilePixelX)) & 1;
    int msb = (vram[tileOffset + lineOffset + 1] >> (7 - tilePixelX)) & 1;
    return static_cast<LCD::Color>((msb << 1) + lsb);
}

LCD::Color PPU::getBGPixelOnScreen(int x, int y)
{
    uint8_t tileId = getBGTileAtScreenPixel(x, y);
    int tilePixelX = x % TILE_LENGTH;
    int tilePixelY = y % TILE_LENGTH;
    return getBGTilePixel(tileId, tilePixelX, tilePixelY);
}

LCD::Color PPU::getScreenPixel(int x, int y)
{
    // TODO WIP
    return getBGPixelOnScreen(x, y);
}
