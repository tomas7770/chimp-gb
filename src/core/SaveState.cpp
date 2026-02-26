#include "SaveState.h"

#include <cstring>

constexpr std::string BESS_STRING = "BESS";
constexpr std::string NAME_STRING = "NAME";
constexpr std::string CHIMPGB_STRING = "ChimpGB";
constexpr std::string INFO_STRING = "INFO";
constexpr std::string CORE_STRING = "CORE";
constexpr uint32_t WRAM_SIZE = 32768;
constexpr uint32_t VRAM_SIZE = 16384;
constexpr uint32_t SRAM_SIZE = 131072;
constexpr uint32_t OAM_SIZE = 160;
constexpr uint32_t HRAM_SIZE = 127;
constexpr std::string MBC_STRING = "MBC ";
constexpr std::string RTC_STRING = "RTC ";
constexpr std::string END_STRING = "END ";

const std::vector<uint8_t> wordToByteArray(uint32_t word)
{
    uint8_t *wordBytes = reinterpret_cast<uint8_t *>(&word);
    return std::vector<uint8_t>{wordBytes[0], wordBytes[1], wordBytes[2], wordBytes[3]};
}

uint32_t byteArrayToWord(const std::vector<uint8_t> &bytes, int offset)
{
    return (bytes.at(offset + 3) << 24) | (bytes.at(offset + 2) << 16) |
           (bytes.at(offset + 1) << 8) | bytes.at(offset);
}

// Check whether data of the given length actually exists after the header
void validateLength(const std::vector<uint8_t> &stateData, int headerOffset, uint32_t length)
{
    if (headerOffset + 8 + length > stateData.size())
    {
        throw std::runtime_error("Invalid save state: block body cut by EOF");
    }
}

// Check whether the block length as reported by the header is the expected
void checkLength(const std::vector<uint8_t> &stateData, int headerOffset, uint32_t correctLength,
                 const std::string &blockName)
{
    uint32_t length = byteArrayToWord(stateData, headerOffset + 4);
    if (length != correctLength)
    {
        throw std::runtime_error("Invalid save state: invalid " + blockName + " length");
    }
    validateLength(stateData, headerOffset, correctLength);
}

void validateMemoryLength(const std::vector<uint8_t> &stateData, int offset, uint32_t length)
{
    if (offset + length > stateData.size())
    {
        throw std::runtime_error("Invalid save state: memory block cut by EOF");
    }
}

void copyMemoryBlock(uint8_t *dest, const std::vector<uint8_t> &stateData,
                     int bodyOffset, int sizeOffset)
{
    uint32_t ramSize = byteArrayToWord(stateData, bodyOffset + sizeOffset);
    uint32_t ramOffset = byteArrayToWord(stateData, bodyOffset + sizeOffset + 4);
    validateMemoryLength(stateData, ramOffset, ramSize);
    memcpy(dest, stateData.data() + ramOffset, ramSize);
}

uint32_t SaveState::parseBlock(const std::vector<uint8_t> &stateData, int headerOffset)
{
    if (headerOffset + 8 > stateData.size())
    {
        throw std::runtime_error("Invalid save state: block header cut by EOF");
    }

    int bodyOffset = headerOffset + 8;
    auto data = stateData.data();
    if (memcmp(data + headerOffset, INFO_STRING.c_str(), 4) == 0)
    {
        checkLength(stateData, headerOffset, 0x12, INFO_STRING);

        for (int i = 0; i < 16; i++)
        {
            titleChars[i] = stateData.at(bodyOffset + i);
        }
        globalChecksum[0] = stateData.at(bodyOffset + 0x10);
        globalChecksum[1] = stateData.at(bodyOffset + 0x11);

        return 0x12;
    }
    else if (memcmp(data + headerOffset, CORE_STRING.c_str(), 4) == 0)
    {
        checkLength(stateData, headerOffset, 0xD0, CORE_STRING);

        uint16_t majorBESSVersion = (stateData.at(bodyOffset + 0x01) << 8) | stateData.at(bodyOffset);
        if (majorBESSVersion != 1)
        {
            throw std::runtime_error("Incompatible save state: major BESS version is " +
                                     std::to_string(majorBESSVersion) + ", supported version is 1");
        }

        uint8_t modelFamily = stateData.at(bodyOffset + 0x04);
        uint8_t model = stateData.at(bodyOffset + 0x05);
        if (modelFamily == 'G' && model == 'D')
        {
            systemType = SystemType::DMG;
        }
        else if (modelFamily == 'C' && model == 'C')
        {
            systemType = SystemType::CGB;
        }
        else
        {
            throw std::runtime_error("Incompatible save state: unsupported console model");
        }

        PC = (stateData.at(bodyOffset + 0x09) << 8) | stateData.at(bodyOffset + 0x08);
        regA = stateData.at(bodyOffset + 0x0A);
        regF = stateData.at(bodyOffset + 0x0B);
        regB = stateData.at(bodyOffset + 0x0C);
        regC = stateData.at(bodyOffset + 0x0D);
        regD = stateData.at(bodyOffset + 0x0E);
        regE = stateData.at(bodyOffset + 0x0F);
        regH = stateData.at(bodyOffset + 0x10);
        regL = stateData.at(bodyOffset + 0x11);
        SP = (stateData.at(bodyOffset + 0x13) << 8) | stateData.at(bodyOffset + 0x12);
        IME = stateData.at(bodyOffset + 0x14) ? true : false;
        IE = stateData.at(bodyOffset + 0x15);
        halted = stateData.at(bodyOffset + 0x16) == 1 ? true : false;
        memcpy(ioRegisters, data + bodyOffset + 0x18, 128);

        copyMemoryBlock(wram, stateData, bodyOffset, 0x98);
        copyMemoryBlock(vram, stateData, bodyOffset, 0xA0);
        copyMemoryBlock(sram, stateData, bodyOffset, 0xA8);
        copyMemoryBlock(oam, stateData, bodyOffset, 0xB0);
        copyMemoryBlock(hram, stateData, bodyOffset, 0xB8);
        copyMemoryBlock(colorBGPaletteMemory, stateData, bodyOffset, 0xC0);
        copyMemoryBlock(colorOBJPaletteMemory, stateData, bodyOffset, 0xC8);

        return 0xD0;
    }
    else if (memcmp(data + headerOffset, MBC_STRING.c_str(), 4) == 0)
    {
        uint32_t length = byteArrayToWord(stateData, headerOffset + 4);
        if (length % 3)
        {
            throw std::runtime_error("Invalid save state: MBC block length must be divisible by 3");
        }
        validateLength(stateData, headerOffset, length);

        for (int i = 0; i < length; i++)
        {
            mbcBlock.push_back(stateData.at(bodyOffset + i));
        }

        return length;
    }
    else if (memcmp(data + headerOffset, RTC_STRING.c_str(), 4) == 0)
    {
        checkLength(stateData, headerOffset, 0x30, RTC_STRING);

        memcpy(rtcBytes, data + bodyOffset, 0x30);

        return 0x30;
    }
    else if (memcmp(data + headerOffset, END_STRING.c_str(), 4) == 0)
    {
        checkLength(stateData, headerOffset, 0x00, END_STRING);
        return 0;
    }
    else
    {
        // Unknown block
        return byteArrayToWord(stateData, headerOffset + 4);
    }
}

SaveState::SaveState(const std::vector<uint8_t> &stateData)
{
    int size = stateData.size();
    if (size < 8)
    {
        throw std::runtime_error("Invalid save state: too small");
    }

    // Look for BESS footer
    if (memcmp(stateData.data() + size - 4, "BESS", 4) != 0)
    {
        throw std::runtime_error("Invalid save state: BESS footer not found");
    }

    uint32_t blockOffset = byteArrayToWord(stateData, size - 8);
    uint32_t blockLength;
    while (blockLength = parseBlock(stateData, blockOffset))
    {
        blockOffset += blockLength + 8;
    }
}

const std::vector<uint8_t> CHIMPGB_STRING_LEN = wordToByteArray(CHIMPGB_STRING.length());

std::shared_ptr<std::vector<uint8_t>> SaveState::serialize() const
{
    auto stateData = std::make_shared<std::vector<uint8_t>>();

    // Implementation-specific

    const std::vector<uint8_t> wramOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), wram, wram + WRAM_SIZE);
    const std::vector<uint8_t> vramOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), vram, vram + VRAM_SIZE);
    const std::vector<uint8_t> sramOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), sram, sram + SRAM_SIZE);
    const std::vector<uint8_t> oamOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), oam, oam + OAM_SIZE);
    const std::vector<uint8_t> hramOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), hram, hram + HRAM_SIZE);
    const std::vector<uint8_t> bgPalOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), colorBGPaletteMemory, colorBGPaletteMemory + 64);
    const std::vector<uint8_t> objPalOffset = wordToByteArray(stateData->size());
    stateData->insert(stateData->end(), colorOBJPaletteMemory, colorOBJPaletteMemory + 64);

    // BESS
    const std::vector<uint8_t> bessOffset = wordToByteArray(stateData->size());

    // NAME block
    stateData->insert(stateData->end(), NAME_STRING.begin(), NAME_STRING.end());
    stateData->insert(stateData->end(), CHIMPGB_STRING_LEN.begin(), CHIMPGB_STRING_LEN.end());
    stateData->insert(stateData->end(), CHIMPGB_STRING.begin(), CHIMPGB_STRING.end());

    // INFO block
    stateData->insert(stateData->end(), INFO_STRING.begin(), INFO_STRING.end());
    stateData->push_back(0x12);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->insert(stateData->end(), titleChars, titleChars + 16);
    stateData->insert(stateData->end(), globalChecksum, globalChecksum + 2);

    // CORE block
    stateData->insert(stateData->end(), CORE_STRING.begin(), CORE_STRING.end());
    stateData->push_back(0xD0);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    // BESS version (1.1)
    stateData->push_back(1);
    stateData->push_back(0);
    stateData->push_back(1);
    stateData->push_back(0);
    switch (systemType)
    {
    case SystemType::DMG:
        stateData->push_back('G');
        stateData->push_back('D');
        stateData->push_back(' ');
        stateData->push_back(' ');
        break;

    case SystemType::CGB:
        stateData->push_back('C');
        stateData->push_back('C');
        stateData->push_back(' ');
        stateData->push_back(' ');
        break;

    default:
        stateData->push_back(' ');
        stateData->push_back(' ');
        stateData->push_back(' ');
        stateData->push_back(' ');
        break;
    }
    stateData->push_back(PC & 0xFF);
    stateData->push_back(PC >> 8);
    stateData->push_back(regA);
    stateData->push_back(regF);
    stateData->push_back(regB);
    stateData->push_back(regC);
    stateData->push_back(regD);
    stateData->push_back(regE);
    stateData->push_back(regH);
    stateData->push_back(regL);
    stateData->push_back(SP & 0xFF);
    stateData->push_back(SP >> 8);
    stateData->push_back(IME ? 1 : 0);
    stateData->push_back(IE);
    stateData->push_back(halted ? 1 : 0);
    stateData->push_back(0); // reserved
    stateData->insert(stateData->end(), ioRegisters, ioRegisters + 128);
    stateData->push_back(0x00);
    stateData->push_back(0x80);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->insert(stateData->end(), wramOffset.begin(), wramOffset.end());
    stateData->push_back(0x00);
    stateData->push_back(0x40);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->insert(stateData->end(), vramOffset.begin(), vramOffset.end());
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->push_back(0x01);
    stateData->push_back(0x00);
    stateData->insert(stateData->end(), sramOffset.begin(), sramOffset.end());
    stateData->push_back(OAM_SIZE);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->insert(stateData->end(), oamOffset.begin(), oamOffset.end());
    stateData->push_back(HRAM_SIZE);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->insert(stateData->end(), hramOffset.begin(), hramOffset.end());
    stateData->push_back(64);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->insert(stateData->end(), bgPalOffset.begin(), bgPalOffset.end());
    stateData->push_back(64);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->push_back(0);
    stateData->insert(stateData->end(), objPalOffset.begin(), objPalOffset.end());

    // MBC block
    stateData->insert(stateData->end(), MBC_STRING.begin(), MBC_STRING.end());
    const std::vector<uint8_t> mbcBlockSize = wordToByteArray(mbcBlock.size());
    stateData->insert(stateData->end(), mbcBlockSize.begin(), mbcBlockSize.end());
    stateData->insert(stateData->end(), mbcBlock.begin(), mbcBlock.end());

    // RTC block
    stateData->insert(stateData->end(), RTC_STRING.begin(), RTC_STRING.end());
    stateData->push_back(0x30);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->insert(stateData->end(), rtcBytes, rtcBytes + 0x30);

    // END block
    stateData->insert(stateData->end(), END_STRING.begin(), END_STRING.end());
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->push_back(0x00);
    stateData->push_back(0x00);

    // BESS footer
    stateData->insert(stateData->end(), bessOffset.begin(), bessOffset.end());
    stateData->insert(stateData->end(), BESS_STRING.begin(), BESS_STRING.end());

    return stateData;
}
