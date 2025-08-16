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
            setMode(1);
            mGameboy->requestInterrupt(CPU::InterruptSource::VBlank);
        }
        else if (mLCD->LY < LCD::SCREEN_H)
        {
            setMode(2);
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

        if (mLCD->LYC == mLCD->LY)
        {
            mLCD->STAT |= LCD::STAT_LYC_LY_BITMASK;
            if (mLCD->STAT & LCD::STAT_LYC_INT_BITMASK)
            {
                mGameboy->requestInterrupt(CPU::InterruptSource::LCD);
            }
        }
        else
        {
            mLCD->STAT &= ~LCD::STAT_LYC_LY_BITMASK;
        }
    }
    else if (mLCD->LY < LCD::SCREEN_H)
    {
        if (mCurrentDot >= MODE_2_DOTS + MODE_3_DOTS)
        {
            setMode(0);
        }
        else if (mCurrentDot >= MODE_2_DOTS)
        {
            setMode(3);
        }
    }
}

void PPU::setMode(int mode)
{
    mMode = mode;
    mLCD->STAT &= ~LCD::STAT_MODE_BITMASK;
    mLCD->STAT |= mode;
}

uint8_t PPU::getBGTileAtScreenPixel(int x, int y)
{
    return vram[TILE_MAP_0_ADDR + (x / TILE_LENGTH) + TILE_MAP_LENGTH * (y / TILE_LENGTH)];
}

int PPU::getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY, bool xFlip, bool yFlip)
{
    // TODO ADDRESSING MODES
    int tileOffset = tileId * TILE_BYTES;
    // Apply flip
    if (xFlip)
    {
        tilePixelX = 7 - tilePixelX;
    }
    if (yFlip)
    {
        tilePixelY = 7 - tilePixelY;
    }
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
    x = (x + mLCD->SCX) % 256;
    y = (y + mLCD->SCY) % 256;
    uint8_t tileId = getBGTileAtScreenPixel(x, y);
    int tilePixelX = x % TILE_LENGTH;
    int tilePixelY = y % TILE_LENGTH;
    return getBGTilePixel(tileId, tilePixelX, tilePixelY);
}

LCD::Color PPU::getPaletteColor(uint8_t palette, int colorId)
{
    switch (colorId)
    {
    default:
    case 0:
        return static_cast<LCD::Color>(palette & 0b11);
    case 1:
        return static_cast<LCD::Color>((palette >> 2) & 0b11);
    case 2:
        return static_cast<LCD::Color>((palette >> 4) & 0b11);
    case 3:
        return static_cast<LCD::Color>((palette >> 6) & 0b11);
    }
}

LCD::Color PPU::getScreenPixel(int pixelX, int pixelY)
{
    // TODO WIP
    int colorId = 0;
    uint8_t palette = 0;
    int bgColorId = 0;
    if (mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_ENABLE)
    {
        bgColorId = getBGPixelOnScreen(pixelX, pixelY);
        colorId = bgColorId;
        palette = mLCD->BGP;
    }
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
        uint8_t flags = oam[i + 3]; // Byte 3: attributes/flags
        int objColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY,
            (flags & OBJ_FLAG_X_FLIP) ? true : false, (flags & OBJ_FLAG_Y_FLIP) ? true : false);
        bool bgHasPriority = (flags & OBJ_FLAG_PRIORITY) && bgColorId > 0;
        if (objColorId != 0 && !bgHasPriority)
        {
            colorId = objColorId;
            if (flags & OBJ_FLAG_DMG_PAL)
            {
                palette = mLCD->OBP1;
            }
            else
            {
                palette = mLCD->OBP0;
            }
        }
    }
    return getPaletteColor(palette, colorId);
}
