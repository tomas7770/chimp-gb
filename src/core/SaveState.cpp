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
