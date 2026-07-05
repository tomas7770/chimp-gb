#include "PPU.h"

#include "Gameboy.h"
#include "LCD.h"
#include "CPU.h"
#include <cstring>

void PPU::writeLCDC(uint8_t value)
{
    if (!(mLCD->LCDC & LCD::LCDC_FLAG_LCD_PPU_ENABLE) && (value & LCD::LCDC_FLAG_LCD_PPU_ENABLE))
    {
        // Enable LCD/PPU
        mEnabled = true;
        mFirstFrameAfterEnable = true;
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
        case SystemType::DMG:
            for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
            {
                mLCD->pixels[i].dmg = LCD::DMGColor::White;
            }
            break;

        case SystemType::CGB:
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
        mGameboy->removeEvent(PPU_OAMScan_End);
        mGameboy->removeEvent(PPU_Draw_End);
        mGameboy->removeEvent(PPU_NewLine);
        mGameboy->removeEvent(PPU_DelayedVBlank);
        mGameboy->removeEvent(PPU_EarlyLYUpdate);
        mIncrementedWindowLine = false;
        mWYTriggered = false;
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

void PPU::writeBGP(uint8_t value)
{
    if (mMode == Draw)
    {
        int column = (mGameboy->cycleCounter - mDrawStartCycle) * 2;
        mBGPWrites[column] = value;
    }
}

void PPU::setDrawCallback(void (*drawCallback)(void *), void *userdata)
{
    this->drawCallback = drawCallback;
    mDrawCallbackUserdata = userdata;
}

void PPU::setMode(PPU::Mode mode)
{
    mMode = mode;
    switch (mMode)
    {
    case OAMScan:
        mGameboy->addEvent(PPU_OAMScan_End, MODE_2_DOTS / 2);
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
        mGameboy->addEvent(PPU_Draw_End, MODE_3_DOTS / 2);
        mDrawStartCycle = mGameboy->cycleCounter;
        mCurrentBGP = mLCD->BGP;
        mBGPWrites.clear();
        break;

    case HBlank:
        mGameboy->addEvent(PPU_NewLine, MODE_0_DOTS / 2);
        break;

    case VBlank:
        mGameboy->addEvent(PPU_NewLine, MODE_1_DOTS / 2);
        if (delayedVBLInterrupt())
        {
            mGameboy->addEvent(PPU_DelayedVBlank, 4 / 2);
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

void PPU::updateLYCInterrupt()
{
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

void PPU::newLine(bool generateFrame)
{
    if (!(mLCD->LY == 0 && mMode == VBlank)) // don't increment if it's the early LY = 0 from scanline 153
    {
        mLCD->LY++;
        mLCD->LY %= LCD::SCREEN_H + VBLANK_LINES;
    }
    if (mLCD->LY == LCD::SCREEN_H)
    {
        mLCD->windowLineCounter = 0;
        mWYTriggered = false;
        setMode(VBlank);
        if (!delayedVBLInterrupt())
        {
            doVBLInterrupt();
        }
        if (mFirstFrameAfterEnable)
        {
            mFirstFrameAfterEnable = false;
        }
        else if (drawCallback != nullptr && generateFrame)
        {
            drawCallback(mDrawCallbackUserdata);
        }
    }
    else if (mLCD->LY < LCD::SCREEN_H)
    {
        setMode(OAMScan);
        mIncrementedWindowLine = false;
        if (!mWYTriggered && mLCD->WY == mLCD->LY)
        {
            mWYTriggered = true;
        }
    }
    else if (mLCD->LY > LCD::SCREEN_H)
    {
        mGameboy->addEvent(PPU_NewLine, MODE_1_DOTS / 2);
        if (mLCD->LY == (LCD::SCREEN_H + VBLANK_LINES - 1))
        {
            mGameboy->addEvent(PPU_EarlyLYUpdate, 4 / 2);
        }
    }

    updateLYCInterrupt();
}

bool PPU::delayedVBLInterrupt()
{
    return !(mGameboy->inCGBMode()) || mGameboy->isCPUDoubleSpeed();
}

void PPU::doVBLInterrupt()
{
    mGameboy->requestInterrupt(CPU::InterruptSource::VBlank);
}

void PPU::eventOAMScanEnd()
{
    setMode(Draw);
}

void PPU::eventDrawEnd(bool generateFrame)
{
    // Draw scanline
    if (generateFrame)
    {
        updateScreenPixels(mLCD->LY);
    }
    else
    {
        bool bgEnable = mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_ENABLE;
        if (mGameboy->inCGBMode() || bgEnable)
        {
            if ((mLCD->LCDC & LCD::LCDC_FLAG_WINDOW_ENABLE) && mWYTriggered && mLCD->WX - 7 < LCD::SCREEN_W)
            {
                mLCD->windowLineCounter++;
            }
        }
    }
    setMode(HBlank);
}

void PPU::eventNewLine(bool generateFrame)
{
    newLine(generateFrame);
}

void PPU::eventDelayedVBlank()
{
    doVBLInterrupt();
}

void PPU::eventEarlyLYUpdate()
{
    // LY is updated early (original hardware quirk)
    mLCD->LY = 0;
    updateLYCInterrupt();
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

inline LCD::DMGColor getDMGPaletteColor(uint8_t paletteByte, int colorId)
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

inline uint8_t *getCGBPalette(int paletteIndex, uint8_t *paletteMemory)
{
    return paletteMemory + paletteIndex * 8;
}

inline LCD::CGBColor getCGBPaletteColor(uint8_t *palette, int colorId)
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

inline LCD::CGBColor getDMGCompatPaletteColor(uint8_t *cgbPalette, uint8_t dmgPaletteByte, int colorId)
{
    switch (colorId)
    {
    default:
    case 0:
        return getCGBPaletteColor(cgbPalette, dmgPaletteByte & 0b11);
    case 1:
        return getCGBPaletteColor(cgbPalette, (dmgPaletteByte >> 2) & 0b11);
    case 2:
        return getCGBPaletteColor(cgbPalette, (dmgPaletteByte >> 4) & 0b11);
    case 3:
        return getCGBPaletteColor(cgbPalette, (dmgPaletteByte >> 6) & 0b11);
    }
}

void PPU::drawPixel(int pixelCoord, SystemType systemType, bool cgbMode, int colorId,
                    uint8_t *palette, uint8_t dmgPaletteByte)
{
    switch (systemType)
    {
    case SystemType::DMG:
        if (palette == nullptr)
        {
            mLCD->pixels[pixelCoord] = {.dmg = LCD::DMGColor::White};
        }
        else
        {
            mLCD->pixels[pixelCoord] = {.dmg = getDMGPaletteColor(*palette, colorId)};
        }
        break;

    case SystemType::CGB:
        if (palette == nullptr)
        {
            mLCD->pixels[pixelCoord] = {.cgb = {.r = 31, .g = 31, .b = 31}};
        }
        else if (cgbMode)
        {
            mLCD->pixels[pixelCoord] = {.cgb = getCGBPaletteColor(palette, colorId)};
        }
        else
        {
            mLCD->pixels[pixelCoord] = {.cgb = getDMGCompatPaletteColor(palette, dmgPaletteByte, colorId)};
        }
        break;

    default:
        break;
    }
}

void PPU::drawBGTileRow(uint8_t tileId, uint8_t attributes, int tileStart, int tileEnd, int row,
                        int pixelX, int pixelY, SystemType systemType, bool cgbMode)
{
    // If there are any BGP writes in the middle of the tile, clear the tile cache and don't use it.
    // This obviously affects performance, but most games don't do such writes.
    bool useTileCache = true;
    for (const auto &paletteWrite : mBGPWrites)
    {
        if (paletteWrite.first >= pixelX + tileStart && paletteWrite.first <= pixelX + tileEnd)
        {
            mBGTileCache.clear();
            useTileCache = false;
            break;
        }
    }

    if (useTileCache && tileStart == 0 && tileEnd == TILE_LENGTH - 1)
    {
        auto attributesTileIdKey = (attributes << 8) | tileId;
        for (auto &cachedTile : mBGTileCache)
        {
            if (cachedTile.attributesTileIdKey == attributesTileIdKey)
            {
                int srcPixelX = cachedTile.pixelX;
                int destOffset = pixelY * LCD::SCREEN_W + pixelX;
                int srcOffset = pixelY * LCD::SCREEN_W + srcPixelX;
                memcpy(mLCD->pixels + destOffset, mLCD->pixels + srcOffset, sizeof(LCD::Color) * TILE_LENGTH);
                memcpy(mBGColorIdCache + pixelX, mBGColorIdCache + srcPixelX, sizeof(int) * TILE_LENGTH);
                memcpy(mBGForcePriorityCache + pixelX, mBGForcePriorityCache + srcPixelX, sizeof(bool) * TILE_LENGTH);
                return;
            }
        }
        mBGTileCache.push_back(CachedTile{.attributesTileIdKey = attributesTileIdKey, .pixelX = pixelX});
    }

    bool xFlip = false;
    bool yFlip = false;
    int bank = 0;
    int paletteIndex;
    bool bgForcePriority = false;
    if (cgbMode)
    {
        xFlip = attributes & BG_ATTRIB_FLAG_X_FLIP;
        yFlip = attributes & BG_ATTRIB_FLAG_Y_FLIP;
        bank = (attributes & BG_ATTRIB_FLAG_BANK) >> 3;
        paletteIndex = attributes & CGB_PAL_BITMASK;
        bgForcePriority = (attributes & BG_ATTRIB_FLAG_PRIORITY) ? true : false;
    }

    int tileOffset;
    if (mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_TILE_DATA)
    {
        tileOffset = tileId * TILE_BYTES;
    }
    else
    {
        tileOffset = TILE_BLOCK_2_OFFSET + static_cast<int8_t>(tileId) * TILE_BYTES;
    }
    // Apply flip
    if (yFlip)
    {
        row = 7 - row;
    }
    // Each tile occupies 16 bytes, where each line is represented by 2 bytes.
    int lineOffset = row * 2;
    // The first byte specifies the LSB of the color ID of each pixel, and the second byte specifies the MSB.
    int lsbRow = vram[tileOffset + lineOffset + bank * VRAM_BANK_SIZE];
    int msbRow = vram[tileOffset + lineOffset + 1 + bank * VRAM_BANK_SIZE];

    for (int i = tileStart; i <= tileEnd; i++)
    {
        int tilePixelX = i;
        if (xFlip)
        {
            tilePixelX = 7 - tilePixelX;
        }
        int lsb = (lsbRow >> (7 - tilePixelX)) & 1;
        int msb = (msbRow >> (7 - tilePixelX)) & 1;
        mBGColorIdCache[pixelX] = (msb << 1) + lsb;
        int colorId = mBGColorIdCache[pixelX];
        mBGForcePriorityCache[pixelX] = bgForcePriority;

        int pixelCoord = pixelY * LCD::SCREEN_W + pixelX;

        // Get palette
        uint8_t *palette = nullptr;
        uint8_t dmgPaletteByte;
        switch (systemType)
        {
        case SystemType::DMG:
            if (mBGPWrites.contains(pixelX + i))
            {
                mCurrentBGP = mBGPWrites[pixelX + i];
            }
            palette = &(mCurrentBGP);
            break;

        case SystemType::CGB:
            if (cgbMode)
            {
                palette = getCGBPalette(paletteIndex, mLCD->colorBGPaletteMemory);
            }
            else
            {
                palette = getCGBPalette(0, mLCD->colorBGPaletteMemory);
                if (mBGPWrites.contains(pixelX + i))
                {
                    mCurrentBGP = mBGPWrites[pixelX + i];
                }
                dmgPaletteByte = mCurrentBGP;
            }
            break;

        default:
            break;
        }

        drawPixel(pixelCoord, systemType, cgbMode, colorId, palette, dmgPaletteByte);

        pixelX++;
    }
}

void PPU::updateScreenPixels(int pixelY)
{
    int pixelCoord = pixelY * LCD::SCREEN_W;

    SystemType systemType = mGameboy->getSystemType();
    bool cgbMode = mGameboy->inCGBMode();
    bool bgEnable = mLCD->LCDC & LCD::LCDC_FLAG_BG_WINDOW_ENABLE;

    if (cgbMode || bgEnable)
    {
        mBGTileCache.clear();
        int bgEndX = LCD::SCREEN_W;
        if ((mLCD->LCDC & LCD::LCDC_FLAG_WINDOW_ENABLE) && mWYTriggered && mLCD->WX - 7 < LCD::SCREEN_W)
        {
            if (!mIncrementedWindowLine)
            {
                mLCD->windowLineCounter++;
                mIncrementedWindowLine = true;
            }
            bgEndX = mLCD->WX - 7;
        }

        // Background
        int y = (pixelY + mLCD->SCY) % 256;
        uint16_t tileMapAddr;
        if (mLCD->LCDC & LCD::LCDC_FLAG_BG_TILE_MAP)
        {
            tileMapAddr = TILE_MAP_1_ADDR;
        }
        else
        {
            tileMapAddr = TILE_MAP_0_ADDR;
        }
        uint16_t tileIdAddr = tileMapAddr + (mLCD->SCX / TILE_LENGTH) + TILE_MAP_LENGTH * (y / TILE_LENGTH);
        uint8_t tileId = vram[tileIdAddr];
        uint8_t attributes = vram[tileIdAddr + VRAM_BANK_SIZE];
        drawBGTileRow(tileId, attributes, mLCD->SCX % TILE_LENGTH, TILE_LENGTH - 1, y % TILE_LENGTH,
                      0, pixelY, systemType, cgbMode);
        for (int pixelX = TILE_LENGTH - mLCD->SCX % TILE_LENGTH; pixelX < bgEndX; pixelX += TILE_LENGTH)
        {
            tileIdAddr = tileMapAddr + ((mLCD->SCX + pixelX) % 256 / TILE_LENGTH) + TILE_MAP_LENGTH * (y / TILE_LENGTH);
            tileId = vram[tileIdAddr];
            attributes = vram[tileIdAddr + VRAM_BANK_SIZE];
            int tileEnd = std::min(TILE_LENGTH - 1, bgEndX - pixelX - 1);
            drawBGTileRow(tileId, attributes, 0, tileEnd, y % TILE_LENGTH,
                          pixelX, pixelY, systemType, cgbMode);
        }

        // Window
        mBGTileCache.clear();
        y = mLCD->windowLineCounter - 1;
        if (mLCD->LCDC & LCD::LCDC_FLAG_WINDOW_TILE_MAP)
        {
            tileMapAddr = TILE_MAP_1_ADDR;
        }
        else
        {
            tileMapAddr = TILE_MAP_0_ADDR;
        }
        if (bgEndX < 0)
        {
            tileIdAddr = tileMapAddr + TILE_MAP_LENGTH * (y / TILE_LENGTH);
            tileId = vram[tileIdAddr];
            attributes = vram[tileIdAddr + VRAM_BANK_SIZE];
            drawBGTileRow(tileId, attributes, -bgEndX, TILE_LENGTH - 1, y % TILE_LENGTH,
                          0, pixelY, systemType, cgbMode);
        }
        for (int pixelX = std::max(0, bgEndX); pixelX < LCD::SCREEN_W; pixelX += TILE_LENGTH)
        {
            tileIdAddr = tileMapAddr + ((pixelX - mLCD->WX + 7) / TILE_LENGTH) + TILE_MAP_LENGTH * (y / TILE_LENGTH);
            tileId = vram[tileIdAddr];
            attributes = vram[tileIdAddr + VRAM_BANK_SIZE];
            int tileEnd = std::min(TILE_LENGTH - 1, LCD::SCREEN_W - pixelX - 1);
            drawBGTileRow(tileId, attributes, 0, tileEnd, y % TILE_LENGTH,
                          pixelX, pixelY, systemType, cgbMode);
        }
    }
    else
    {
        for (int pixelX = 0; pixelX < LCD::SCREEN_W; pixelX++)
        {
            drawPixel(pixelCoord, systemType, cgbMode, 0, nullptr, 0);
            pixelCoord++;
        }
    }

    if (mLCD->LCDC & LCD::LCDC_FLAG_OBJ_ENABLE)
    {
        pixelCoord = pixelY * LCD::SCREEN_W;
        for (int pixelX = 0; pixelX < LCD::SCREEN_W; pixelX++)
        {
            int colorId = 0;
            uint8_t *palette = nullptr;
            uint8_t dmgPaletteByte;

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
                if (!cgbMode && (oam[i + 1] > lowestX))
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
                int bank = cgbMode ? ((flags & OBJ_FLAG_BANK) >> 3) : 0;
                int objColorId = getBGTilePixel(tileId, tilePixelX, tilePixelY, true,
                                                (flags & OBJ_FLAG_X_FLIP) ? true : false, yFlip, bank);
                bool bgHasPriority = ((flags & OBJ_FLAG_PRIORITY) || mBGForcePriorityCache[pixelX]) &&
                                     mBGColorIdCache[pixelX] > 0 && bgEnable;
                if (objColorId != 0 && !bgHasPriority)
                {
                    colorId = objColorId;
                    switch (systemType)
                    {
                    case SystemType::DMG:
                        palette = (flags & OBJ_FLAG_DMG_PAL) ? &(mLCD->OBP1) : &(mLCD->OBP0);
                        break;

                    case SystemType::CGB:
                        if (cgbMode)
                        {
                            palette = getCGBPalette(flags & CGB_PAL_BITMASK, mLCD->colorOBJPaletteMemory);
                        }
                        else
                        {
                            palette = getCGBPalette((flags & OBJ_FLAG_DMG_PAL) ? 1 : 0, mLCD->colorOBJPaletteMemory);
                            dmgPaletteByte = (flags & OBJ_FLAG_DMG_PAL) ? mLCD->OBP1 : mLCD->OBP0;
                        }
                        break;

                    default:
                        break;
                    }
                    lowestX = oam[i + 1];

                    drawPixel(pixelCoord, systemType, cgbMode, colorId, palette, dmgPaletteByte);
                }
            }

            pixelCoord++;
        }
    }
}
