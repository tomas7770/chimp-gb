#include "PPU.h"

#include "Gameboy.h"
#include "LCD.h"
#include "CPU.h"

PPU::Mode PPU::getMode() const
{
    return mMode;
}

void PPU::writeLCDC(uint8_t value)
{
    if (!(mLCD->LCDC & LCD::LCDC_FLAG_LCD_PPU_ENABLE) && (value & LCD::LCDC_FLAG_LCD_PPU_ENABLE))
    {
        // Enable LCD/PPU
        mEnabled = true;
        setMode(OAMScan);
    }
    else if ((mLCD->LCDC & LCD::LCDC_FLAG_LCD_PPU_ENABLE) && !(value & LCD::LCDC_FLAG_LCD_PPU_ENABLE))
    {
        // Disable LCD/PPU
        mEnabled = false;
        mLCD->STAT &= ~LCD::STAT_MODE_BITMASK;
        // Clear LCD
        for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
        {
            mLCD->pixels[i] = LCD::Color::White;
        }
        if (drawCallback != nullptr)
        {
            drawCallback(mDrawCallbackUserdata);
        }
        // Reset registers
        mScanlineDots = 0;
        mIncrementedWindowLine = false;
        mStatInterruptLine = 0;
        mLCD->LY = 0;
        mLCD->windowLineCounter = 0;
    }
    mLCD->LCDC = value;
}

void PPU::writeSTAT(uint8_t value)
{
    mLCD->STAT &= LCD::STAT_LYC_LY_BITMASK & LCD::STAT_MODE_BITMASK;
    mLCD->STAT |= (value & ~(LCD::STAT_LYC_LY_BITMASK & LCD::STAT_MODE_BITMASK));
    updateStatInterruptLine();
}

void PPU::writeLYC(uint8_t value)
{
    mLCD->LYC = value;
    updateStatInterruptLine();
}

void PPU::setDrawCallback(void (*drawCallback)(void *), void *userdata)
{
    this->drawCallback = drawCallback;
    mDrawCallbackUserdata = userdata;
}

void PPU::doCycle()
{
    if (!mEnabled)
    {
        return;
    }

    mScanlineDots++;

    switch (mMode)
    {
    case HBlank:
    case VBlank:
        if (mScanlineDots >= DOTS_PER_LINE)
        {
            newLine();
        }
        break;

    case OAMScan:
        if (mScanlineDots >= MODE_2_DOTS)
        {
            setMode(Draw);
        }
        break;

    case Draw:
        // TODO proper FIFO logic
        // Fetch
        if ((mFetcherDots % 2) && !mPendingPush)
        {
            switch (mFetcherState)
            {
            case GetTile:
                mFetcherY = (mLCD->LY + mLCD->SCY) % (TILE_LENGTH * TILE_MAP_LENGTH);
                mCurrentTileOffset = getTileDataOffset(getCurrentBGTile(false), false);
                mFetcherX++;

                mFetcherState = GetTileDataLow;
                break;

            case GetTileDataLow:
                mTileDataLow = vram[mCurrentTileOffset];

                mFetcherState = GetTileDataHigh;
                break;

            case GetTileDataHigh:
                mTileDataHigh = vram[mCurrentTileOffset + 1];
                mPendingPush = true;

                mFetcherState = GetTile;
                break;

            default:
                break;
            }
        }
        mFetcherDots++;
        if (mPendingPush && mBgFifo.empty())
        {
            pushPixelsToFifo();
            mPendingPush = false;
            mFetcherDots = 1;
        }

        // Draw
        if (!mBgFifo.empty())
        {
            const FifoPixel &pixel = mBgFifo.front();
            mBgFifo.pop();

            if (mCurrentX >= 0)
            {
                int pixelCoord = mLCD->LY * LCD::SCREEN_W + mCurrentX;
                mLCD->pixels[pixelCoord] = getPaletteColor(mLCD->BGP, pixel.color);
            }
            mCurrentX++;
        }
        if (mCurrentX >= LCD::SCREEN_W)
        {
            setMode(HBlank);
        }
        break;

    default:
        break;
    }
}

void PPU::pushPixelsToFifo()
{
    // The first byte specifies the LSB of the color ID of each pixel, and the second byte specifies the MSB.
    // Bit 7 represents the leftmost pixel, and bit 0 the rightmost.
    for (int tilePixelX = 0; tilePixelX < 8; tilePixelX++)
    {
        int lsb = (mTileDataLow >> (7 - tilePixelX)) & 1;
        int msb = (mTileDataHigh >> (7 - tilePixelX)) & 1;
        FifoPixel pixel;
        pixel.color = (msb << 1) + lsb;
        mBgFifo.push(pixel);
    }
}

void PPU::setMode(PPU::Mode mode)
{
    mMode = mode;
    switch (mMode)
    {
    case OAMScan:
        spritesInScanline.clear();
        for (int i = 0; i < oamSize; i += SPRITE_BYTES)
        {
            int y = oam[i] - 16; // Byte 0: y+16 (value=0 means y=-16)
            if (mLCD->LY < y || mLCD->LY >= y + ((mLCD->LCDC & LCD::LCDC_FLAG_OBJ_SIZE) ? 16 : 8))
            {
                continue;
            }
            spritesInScanline.push_back(i);
            if (spritesInScanline.size() >= MAX_SPRITES_PER_LINE)
            {
                break;
            }
        }
        break;

    case Draw:
        // TODO init FIFO
        mFetcherDots = 0;
        mBgFifo = std::queue<FifoPixel>();
        mCurrentX = -TILE_LENGTH - (mLCD->SCX % 8);
        mFetcherState = GetTile;
        mFetcherX = -1;
        mPendingPush = false;
        break;

    default:
        break;
    }
    mLCD->STAT &= ~LCD::STAT_MODE_BITMASK;
    mLCD->STAT |= mode;
    updateStatInterruptLine();
}

void PPU::updateStatInterruptLine()
{
    int line = 0;
    if ((mLCD->STAT & LCD::STAT_LYC_INT_BITMASK) && (mLCD->LYC == mLCD->LY))
    {
        line |= LCD::STAT_LYC_INT_BITMASK;
    }
    switch (mMode)
    {
    case 0:
        if (mLCD->STAT & LCD::STAT_MODE_0_INT_BITMASK)
        {
            line |= LCD::STAT_MODE_0_INT_BITMASK;
        }
        break;
    case 1:
        if (mLCD->STAT & LCD::STAT_MODE_1_INT_BITMASK)
        {
            line |= LCD::STAT_MODE_1_INT_BITMASK;
        }
        break;
    case 2:
        if (mLCD->STAT & LCD::STAT_MODE_2_INT_BITMASK)
        {
            line |= LCD::STAT_MODE_2_INT_BITMASK;
        }
        break;

    default:
        break;
    }

    if (!mStatInterruptLine && line)
    {
        mGameboy->requestInterrupt(CPU::InterruptSource::LCD);
    }
    mStatInterruptLine = line;
}

void PPU::newLine()
{
    mScanlineDots = 0;
    mLCD->LY++;
    mLCD->LY %= LCD::SCREEN_H + VBLANK_LINES;
    if (mLCD->LY == LCD::SCREEN_H)
    {
        mLCD->windowLineCounter = 0;
        setMode(VBlank);
        mGameboy->requestInterrupt(CPU::InterruptSource::VBlank);
        if (drawCallback != nullptr)
        {
            drawCallback(mDrawCallbackUserdata);
        }
    }
    else if (mLCD->LY < LCD::SCREEN_H)
    {
        setMode(OAMScan);
        mIncrementedWindowLine = false;
    }

    if (mLCD->LYC == mLCD->LY)
    {
        mLCD->STAT |= LCD::STAT_LYC_LY_BITMASK;
    }
    else
    {
        mLCD->STAT &= ~LCD::STAT_LYC_LY_BITMASK;
    }
    updateStatInterruptLine();
}

uint8_t PPU::getCurrentBGTile(bool isWindow) const
{
    uint16_t tileMapAddr;
    if (mLCD->LCDC & (isWindow ? LCD::LCDC_FLAG_WINDOW_TILE_MAP : LCD::LCDC_FLAG_BG_TILE_MAP))
    {
        tileMapAddr = TILE_MAP_1_ADDR;
    }
    else
    {
        tileMapAddr = TILE_MAP_0_ADDR;
    }
    int x = ((mLCD->SCX / TILE_LENGTH) + mFetcherX) % TILE_MAP_LENGTH;
    return vram[tileMapAddr + x + TILE_MAP_LENGTH * (mFetcherY / TILE_LENGTH)];
}

int PPU::getTileDataOffset(uint8_t tileId, bool drawingObj) const
{
    int tileOffset;
    if ((mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_TILE_DATA) || drawingObj)
    {
        tileOffset = tileId * TILE_BYTES;
    }
    else
    {
        tileOffset = TILE_BLOCK_2_OFFSET + static_cast<int8_t>(tileId) * TILE_BYTES;
    }
    // Each tile occupies 16 bytes, where each line is represented by 2 bytes.
    int lineOffset = (mFetcherY % TILE_LENGTH) * 2;
    return tileOffset + lineOffset;
}

int PPU::getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY, bool drawingObj, bool xFlip, bool yFlip)
{
    int tileOffset;
    if ((mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_TILE_DATA) || drawingObj)
    {
        tileOffset = tileId * TILE_BYTES;
    }
    else
    {
        tileOffset = TILE_BLOCK_2_OFFSET + static_cast<int8_t>(tileId) * TILE_BYTES;
    }
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

// int PPU::getBGPixelOnScreen(int x, int y)
// {
//     x = (x + mLCD->SCX) % 256;
//     y = (y + mLCD->SCY) % 256;
//     uint8_t tileId = getBGTileAtScreenPixel(x, y, false);
//     int tilePixelX = x % TILE_LENGTH;
//     int tilePixelY = y % TILE_LENGTH;
//     return getBGTilePixel(tileId, tilePixelX, tilePixelY, false);
// }

// int PPU::getWindowPixel(int x, int y)
// {
//     uint8_t tileId = getBGTileAtScreenPixel(x, y, true);
//     int tilePixelX = x % TILE_LENGTH;
//     int tilePixelY = y % TILE_LENGTH;
//     return getBGTilePixel(tileId, tilePixelX, tilePixelY, false);
// }

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

// LCD::Color PPU::getScreenPixel(int pixelX, int pixelY)
// {
//     int colorId = 0;
//     uint8_t palette = 0;
//     int bgColorId = 0;
//     if (mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_ENABLE)
//     {
//         if ((mLCD->LCDC & LCD::LCDC_FLAG_WINDOW_ENABLE) && pixelX >= mLCD->WX - 7 && pixelY >= mLCD->WY)
//         {
//             if (!mIncrementedWindowLine)
//             {
//                 mLCD->windowLineCounter++;
//                 mIncrementedWindowLine = true;
//             }
//             bgColorId = getWindowPixel(pixelX - mLCD->WX + 7, mLCD->windowLineCounter - 1);
//         }
//         else
//         {
//             bgColorId = getBGPixelOnScreen(pixelX, pixelY);
//         }
//         colorId = bgColorId;
//         palette = mLCD->BGP;
//     }
//     if (mLCD->LCDC & LCD::LCDC_FLAG_OBJ_ENABLE)
//     {
//         // The smaller the X coordinate, the higher the object priority.
//         // 256 > any unsigned byte
//         int lowestX = 256;
//         for (int j = spritesInScanline.size() - 1; j >= 0; j--)
//         {
//             // The object located first in OAM has higher priority, so start by the last and let the earlier ones override
//             int i = spritesInScanline.at(j);
//             // Only x is checked because y was already checked during OAM scan
//             int x = oam[i + 1] - 8; // Byte 1: x+8 (value=0 means x=-8)
//             if (pixelX < x || pixelX >= x + 8)
//             {
//                 continue;
//             }

//             // Pixel is within this object
//             if (oam[i + 1] > lowestX)
//             {
//                 continue;
//             }
//             int y = oam[i] - 16; // Byte 0: y+16 (value=0 means y=-16)

//             uint8_t flags = oam[i + 3]; // Byte 3: attributes/flags
//             bool yFlip = (flags & OBJ_FLAG_Y_FLIP) ? true : false;
//             uint8_t tileId = oam[i + 2]; // Byte 2: tile index
//             if (mLCD->LCDC & LCD::LCDC_FLAG_OBJ_SIZE)
//             {
//                 if (yFlip)
//                 {
//                     tileId |= 1;
//                     tileId -= ((pixelY - y) >= 8 ? 1 : 0);
//                 }
//                 else
//                 {
//                     tileId &= ~1;
//                     tileId += ((pixelY - y) >= 8 ? 1 : 0);
//                 }
//             }

//             int tilePixelX = (pixelX - x) % TILE_LENGTH;
//             int tilePixelY = (pixelY - y) % TILE_LENGTH;
//             int objColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY, true,
//                                             (flags & OBJ_FLAG_X_FLIP) ? true : false, yFlip);
//             bool bgHasPriority = (flags & OBJ_FLAG_PRIORITY) && bgColorId > 0;
//             if (objColorId != 0 && !bgHasPriority)
//             {
//                 colorId = objColorId;
//                 if (flags & OBJ_FLAG_DMG_PAL)
//                 {
//                     palette = mLCD->OBP1;
//                 }
//                 else
//                 {
//                     palette = mLCD->OBP0;
//                 }
//                 lowestX = oam[i + 1];
//             }
//         }
//     }
//     return getPaletteColor(palette, colorId);
// }
