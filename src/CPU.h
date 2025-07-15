#pragma once

#include <cstdint>
#include <memory>

class Gameboy;

class CPU
{
public:
    void fetchDecodeExecuteOpcode();

    // DEBUG/TESTING
    struct CPUState
    {
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
        // SP and PC
        uint16_t SP;
        uint16_t PC;
    };
    CPUState getState() const;

private:
    uint16_t getImm16() const;
    uint8_t getImm8() const;

    uint8_t readR8(uint8_t bitmask) const;
    void writeR8(uint8_t bitmask, uint8_t value);
    uint16_t readR16(uint8_t bitmask) const;
    void writeR16(uint8_t bitmask, uint16_t value);
    uint16_t readR16Mem(uint8_t bitmask);
    bool checkCond(uint8_t bitmask) const;
    uint8_t readR16StkLow(uint8_t bitmask) const;
    uint8_t readR16StkHigh(uint8_t bitmask) const;
    void writeR16StkLow(uint8_t bitmask, uint8_t value);
    void writeR16StkHigh(uint8_t bitmask, uint8_t value);

    std::shared_ptr<Gameboy> mGameboy;

    // AF
    uint8_t mRegA;
    uint8_t mRegF;
    // BC
    uint8_t mRegB;
    uint8_t mRegC;
    // DE
    uint8_t mRegD;
    uint8_t mRegE;
    // HL
    uint8_t mRegH;
    uint8_t mRegL;
    // SP and PC
    uint16_t mSP;
    uint16_t mPC = 0x0101; // always ahead (+1) of the currently executed instruction

    static constexpr uint8_t FLAG_ZERO = (1 << 7);
    static constexpr uint8_t FLAG_SUB = (1 << 6);
    static constexpr uint8_t FLAG_HALFCARRY = (1 << 5);
    static constexpr uint8_t FLAG_CARRY = (1 << 4);
};
