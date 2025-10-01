#include "PPU.h"

#include "Gameboy.h"
#include "LCD.h"
#include "CPU.h"
#include <cstring>

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
        switch (mGameboy->getSystemType())
        {
        case Gameboy::SystemType::DMG:
            for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
            {
                mLCD->pixels[i].dmg = LCD::DMGColor::White;
            }
            break;

        case Gameboy::SystemType::CGB:
            for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
            {
                mLCD->pixels[i].cgb = {.r = 31, .g = 31, .b = 31};
            }
            break;

        default:
            break;
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
        if (mScanlineDots >= MODE_2_DOTS + MODE_3_DOTS)
        {
            setMode(HBlank);
        }
        else if (mScanlineDots >= MODE_2_DOTS + MODE_3_DUMMY_DOTS)
        {
            // Draw pixel
            int pixelX = mScanlineDots - MODE_2_DOTS - MODE_3_DUMMY_DOTS;
            int pixelY = mLCD->LY;
            int pixelCoord = pixelY * LCD::SCREEN_W + pixelX;
            mLCD->pixels[pixelCoord] = getScreenPixel(pixelX, pixelY);
        }
        break;

    default:
        break;
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

uint8_t PPU::getBGTileAtScreenPixel(int x, int y, bool isWindow, bool doGetAttributes)
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
    return vram[tileMapAddr + (x / TILE_LENGTH) + TILE_MAP_LENGTH * (y / TILE_LENGTH) + (doGetAttributes ? VRAM_BANK_SIZE : 0)];
}

int PPU::getBGTilePixel(uint8_t tileId, int tilePixelX, int tilePixelY, bool drawingObj, bool xFlip, bool yFlip, int bank)
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
    int lsb = (vram[tileOffset + lineOffset + bank * VRAM_BANK_SIZE] >> (7 - tilePixelX)) & 1;
    int msb = (vram[tileOffset + lineOffset + 1 + bank * VRAM_BANK_SIZE] >> (7 - tilePixelX)) & 1;
    return (msb << 1) + lsb;
}

LCD::DMGColor getDMGPaletteColor(uint8_t paletteByte, int colorId)
{
    switch (colorId)
    {
    default:
    case 0:
        return static_cast<LCD::DMGColor>(paletteByte & 0b11);
    case 1:
        return static_cast<LCD::DMGColor>((paletteByte >> 2) & 0b11);
    case 2:
        return static_cast<LCD::DMGColor>((paletteByte >> 4) & 0b11);
    case 3:
        return static_cast<LCD::DMGColor>((paletteByte >> 6) & 0b11);
    }
}

uint8_t *getCGBPalette(int paletteIndex, uint8_t *paletteMemory)
{
    return paletteMemory + paletteIndex * 8;
}

LCD::CGBColor getCGBPaletteColor(uint8_t *palette, int colorId)
{
    // Palette is 8 bytes: 4 colors x 2 bytes per color
    uint8_t *color = palette + colorId * 2;
    // Little-endian RGB555:
    // - Low byte is Red + lower 3 bits of Green
    // - High byte is upper 2 bits of Green + Blue (+1 dummy byte)
    uint8_t colorLow = *color;
    uint8_t colorHigh = *(color + 1);
    int r = colorLow & 0b11111;
    int g = ((colorHigh & 0b11) << 3) | (colorLow >> 5);
    int b = (colorHigh >> 2) & 0b11111;
    return {.r = r, .g = g, .b = b};
}

LCD::Color PPU::getScreenPixel(int pixelX, int pixelY)
{
    Gameboy::SystemType systemType = mGameboy->getSystemType();

    int colorId = 0;
    uint8_t *palette = nullptr;
    int bgColorId = 0;
    if (mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_ENABLE)
    {
        int x, y;
        bool isWindow;
        if ((mLCD->LCDC & LCD::LCDC_FLAG_WINDOW_ENABLE) && pixelX >= mLCD->WX - 7 && pixelY >= mLCD->WY)
        {
            if (!mIncrementedWindowLine)
            {
                mLCD->windowLineCounter++;
                mIncrementedWindowLine = true;
            }
            x = pixelX - mLCD->WX + 7;
            y = mLCD->windowLineCounter - 1;
            isWindow = true;
        }
        else
        {
            x = (pixelX + mLCD->SCX) % 256;
            y = (pixelY + mLCD->SCY) % 256;
            isWindow = false;
        }

        // Get color and attributes
        uint8_t tileId = getBGTileAtScreenPixel(x, y, isWindow);
        int tilePixelX = x % TILE_LENGTH;
        int tilePixelY = y % TILE_LENGTH;
        int paletteIndex;
        if (mGameboy->inCGBMode())
        {
            uint8_t attributes = getBGTileAtScreenPixel(x, y, isWindow, true);
            bool xFlip = attributes & BG_ATTRIB_FLAG_X_FLIP;
            bool yFlip = attributes & BG_ATTRIB_FLAG_Y_FLIP;
            int bank = (attributes & BG_ATTRIB_FLAG_BANK) >> 3;
            paletteIndex = attributes & CGB_PAL_BITMASK;
            bgColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY, false, xFlip, yFlip, bank);
        }
        else
        {
            bgColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY, false);
        }
        colorId = bgColorId;

        // Get palette
        switch (systemType)
        {
        case Gameboy::SystemType::DMG:
            palette = &(mLCD->BGP);
            break;

        case Gameboy::SystemType::CGB:
            palette = getCGBPalette(paletteIndex, mLCD->colorBGPaletteMemory);
            break;

        default:
            break;
        }
    }
    if (mLCD->LCDC & LCD::LCDC_FLAG_OBJ_ENABLE)
    {
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
            int y = oam[i] - 16; // Byte 0: y+16 (value=0 means y=-16)

            uint8_t flags = oam[i + 3]; // Byte 3: attributes/flags
            bool yFlip = (flags & OBJ_FLAG_Y_FLIP) ? true : false;
            uint8_t tileId = oam[i + 2]; // Byte 2: tile index
            if (mLCD->LCDC & LCD::LCDC_FLAG_OBJ_SIZE)
            {
                if (yFlip)
                {
                    tileId |= 1;
                    tileId -= ((pixelY - y) >= 8 ? 1 : 0);
                }
                else
                {
                    tileId &= ~1;
                    tileId += ((pixelY - y) >= 8 ? 1 : 0);
                }
            }

            int tilePixelX = (pixelX - x) % TILE_LENGTH;
            int tilePixelY = (pixelY - y) % TILE_LENGTH;
            int bank = mGameboy->inCGBMode() ? ((flags & OBJ_FLAG_BANK) >> 3) : 0;
            int objColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY, true,
                                            (flags & OBJ_FLAG_X_FLIP) ? true : false, yFlip, bank);
            bool bgHasPriority = (flags & OBJ_FLAG_PRIORITY) && bgColorId > 0;
            if (objColorId != 0 && !bgHasPriority)
            {
                colorId = objColorId;
                switch (systemType)
                {
                case Gameboy::SystemType::DMG:
                    if (flags & OBJ_FLAG_DMG_PAL)
                    {
                        palette = &(mLCD->OBP1);
                    }
                    else
                    {
                        palette = &(mLCD->OBP0);
                    }
                    break;

                case Gameboy::SystemType::CGB:
                    palette = getCGBPalette(flags & CGB_PAL_BITMASK, mLCD->colorOBJPaletteMemory);
                    break;

                default:
                    break;
                }
                lowestX = oam[i + 1];
            }
        }
    }

    if (palette == nullptr)
    {
        return {};
    }
    switch (systemType)
    {
    case Gameboy::SystemType::DMG:
        return {.dmg = getDMGPaletteColor(*palette, colorId)};

    case Gameboy::SystemType::CGB:
        return {.cgb = getCGBPaletteColor(palette, colorId)};

    default:
        return {};
    }
}
