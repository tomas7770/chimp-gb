#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "SystemType.h"

// Implements the BESS spec
class SaveState
{
public:
    uint8_t titleChars[16];
    uint8_t globalChecksum[2];

    SystemType systemType;

    // PC
    uint16_t PC;
    // AF
    uint8_t regA;
    uint8_t regF;
    // BC
    uint8_t regB;
    uint8_t regC;
    // DE
    uint8_t regD;
    uint8_t regE;
    // HL
    uint8_t regH;
    uint8_t regL;
    // SP
    uint16_t SP;

    bool IME = false;
    uint8_t IE = 0;
    bool halted = false;

    uint8_t ioRegisters[128];

    uint8_t wram[32768];
    uint8_t vram[16384];
    uint8_t sram[131072];
    uint8_t oam[160];
    uint8_t hram[127];
    uint8_t colorBGPaletteMemory[64];
    uint8_t colorOBJPaletteMemory[64];

    std::vector<uint8_t> mbcBlock;
    uint8_t rtcBytes[48];

    SaveState(const std::vector<uint8_t> &stateData);
    SaveState() = default;
    std::shared_ptr<std::vector<uint8_t>> serialize() const;

private:
    uint32_t parseBlock(const std::vector<uint8_t> &stateData, int offset);
};
