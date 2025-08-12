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
            // OAM scan
            spritesInScanline.clear();
            for (int i = 0; i < oamSize; i += SPRITE_BYTES)
            {
                int y = oam[i] - 16; // Byte 0: y+16 (value=0 means y=-16)
                // TODO implement support for 16 pixel tall sprites
                if (mLCD->LY < y || mLCD->LY >= y + 8)
                {
                    continue;
                }
                spritesInScanline.push_back(i);
                if (spritesInScanline.size() >= MAX_SPRITES_PER_LINE)
                {
                    break;
                }
            }
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

int PPU::getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY)
{
    // TODO ADDRESSING MODES
    int tileOffset = tileId * TILE_BYTES;
    // Each tile occupies 16 bytes, where each line is represented by 2 bytes.
    int lineOffset = tilePixelY * 2;
    // The first byte specifies the LSB of the color ID of each pixel, and the second byte specifies the MSB.
    // Bit 7 represents the leftmost pixel, and bit 0 the rightmost.
    int lsb = (vram[tileOffset + lineOffset] >> (7 - tilePixelX)) & 1;
    int msb = (vram[tileOffset + lineOffset + 1] >> (7 - tilePixelX)) & 1;
    return (msb << 1) + lsb;
}

int PPU::getBGPixelOnScreen(int x, int y)
{
    uint8_t tileId = getBGTileAtScreenPixel(x, y);
    int tilePixelX = x % TILE_LENGTH;
    int tilePixelY = y % TILE_LENGTH;
    return getBGTilePixel(tileId, tilePixelX, tilePixelY);
}

LCD::Color PPU::getScreenPixel(int pixelX, int pixelY)
{
    // TODO WIP
    // TODO PALETTES
    int bgColorId = getBGPixelOnScreen(pixelX, pixelY);
    int colorId = bgColorId;
    // The smaller the X coordinate, the higher the object priority.
    // 256 > any unsigned byte
    int lowestX = 256;
    for (int j = spritesInScanline.size() - 1; j >= 0; j--)
    {
        // The object located first in OAM has higher priority, so start by the last and let the earlier ones override
        int i = spritesInScanline.at(j);
        // Only x is checked because y was already checked during OAM scan
        int x = oam[i + 1] - 8; // Byte 1: x+8 (value=0 means x=-8)
        if (pixelX < x || pixelX >= x + 8)
        {
            continue;
        }

        // Pixel is within this object
        if (oam[i + 1] > lowestX)
        {
            continue;
        }
        lowestX = oam[i + 1];
        int y = oam[i] - 16; // Byte 0: y+16 (value=0 means y=-16)
        // TODO implement support for 16 pixel tall sprites (2 tiles)
        uint8_t tileId = oam[i + 2]; // Byte 2: tile index
        int tilePixelX = (pixelX - x) % TILE_LENGTH;
        int tilePixelY = (pixelY - y) % TILE_LENGTH;
        int objColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY);
        uint8_t flags = oam[i + 3]; // Byte 3: attributes/flags
        bool bgHasPriority = (flags & 0b10000000) && bgColorId > 0;
        if (objColorId != 0 && !bgHasPriority)
        {
            colorId = objColorId;
        }
    }
    return static_cast<LCD::Color>(colorId);
}
