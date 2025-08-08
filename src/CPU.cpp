#include "CPU.h"
#include "Gameboy.h"

void CPU::doMCycle()
{
    (*this.*mMCycleFunc)();
}

uint8_t CPU::readByteAtPC() const
{
    return mGameboy->readByte(mPC);
}

uint8_t CPU::readR8(uint8_t bitmask) const
{
    switch (bitmask)
    {
    case 0:
        return mRegB;
    case 1:
        return mRegC;
    case 2:
        return mRegD;
    case 3:
        return mRegE;
    case 4:
        return mRegH;
    case 5:
        return mRegL;
    case 7:
        return mRegA;

    default:
        throw std::runtime_error("Invalid bitmask provided to r8 instruction parameter");
    }
}

void CPU::writeR8(uint8_t bitmask, uint8_t value)
{
    switch (bitmask)
    {
    case 0:
        mRegB = value;
        break;
    case 1:
        mRegC = value;
        break;
    case 2:
        mRegD = value;
        break;
    case 3:
        mRegE = value;
        break;
    case 4:
        mRegH = value;
        break;
    case 5:
        mRegL = value;
        break;
    case 7:
        mRegA = value;
        break;

    default:
        throw std::runtime_error("Invalid bitmask provided to r8 instruction parameter");
    }
}

uint8_t CPU::readByteAtHL() const
{
    return mGameboy->readByte((mRegH << 8) + mRegL);
}

void CPU::writeByteAtHL(uint8_t value)
{
    mGameboy->writeByte((mRegH << 8) + mRegL, value);
}

uint16_t CPU::readR16(uint8_t bitmask) const
{
    switch (bitmask)
    {
    case 0:
        return (mRegB << 8) + mRegC;
    case 1:
        return (mRegD << 8) + mRegE;
    case 2:
        return (mRegH << 8) + mRegL;
    case 3:
        return mSP;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16 instruction parameter");
    }
}

void CPU::writeR16(uint8_t bitmask, uint16_t value)
{
    switch (bitmask)
    {
    case 0:
        mRegB = value >> 8;
        mRegC = (value & 0xFF);
        break;
    case 1:
        mRegD = value >> 8;
        mRegE = (value & 0xFF);
        break;
    case 2:
        mRegH = value >> 8;
        mRegL = (value & 0xFF);
        break;
    case 3:
        mSP = value;
        break;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16 instruction parameter");
    }
}

uint16_t CPU::readR16Mem(uint8_t bitmask)
{
    uint16_t val;
    switch (bitmask)
    {
    case 0:
        return (mRegB << 8) + mRegC;
    case 1:
        return (mRegD << 8) + mRegE;
    case 2:
        val = (mRegH << 8) + mRegL;
        mRegH = (val + 1) >> 8;
        mRegL = (val + 1) & 0xFF;
        return val;
    case 3:
        val = (mRegH << 8) + mRegL;
        mRegH = (val - 1) >> 8;
        mRegL = (val - 1) & 0xFF;
        return val;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16mem instruction parameter");
    }
}

bool CPU::checkCond(uint8_t bitmask) const
{
    switch (bitmask)
    {
    case 0:
        return !(mRegF & FLAG_ZERO);
    case 1:
        return mRegF & FLAG_ZERO;
    case 2:
        return !(mRegF & FLAG_CARRY);
    case 3:
        return mRegF & FLAG_CARRY;

    default:
        throw std::runtime_error("Invalid bitmask provided to cond instruction parameter");
    }
}

uint8_t CPU::readR16StkLow(uint8_t bitmask) const
{
    switch (bitmask)
    {
    case 0:
        return mRegC;
    case 1:
        return mRegE;
    case 2:
        return mRegL;
    case 3:
        return mRegF;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16stk instruction parameter");
    }
}

uint8_t CPU::readR16StkHigh(uint8_t bitmask) const
{
    switch (bitmask)
    {
    case 0:
        return mRegB;
    case 1:
        return mRegD;
    case 2:
        return mRegH;
    case 3:
        return mRegA;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16stk instruction parameter");
    }
}

void CPU::writeR16StkLow(uint8_t bitmask, uint8_t value)
{
    switch (bitmask)
    {
    case 0:
        mRegC = value;
        break;
    case 1:
        mRegE = value;
        break;
    case 2:
        mRegL = value;
        break;
    case 3:
        mRegF = value & 0xF0;
        break;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16stk instruction parameter");
    }
}

void CPU::writeR16StkHigh(uint8_t bitmask, uint8_t value)
{
    switch (bitmask)
    {
    case 0:
        mRegB = value;
        break;
    case 1:
        mRegD = value;
        break;
    case 2:
        mRegH = value;
        break;
    case 3:
        mRegA = value;
        break;

    default:
        throw std::runtime_error("Invalid bitmask provided to r16stk instruction parameter");
    }
}

void CPU::opcode_nop_M1()
{
    prefetchOpcode();
}

void CPU::opcode_ldr8r8_M1()
{
    writeR8((mOpcode & 0b00111000) >> 3, readR8(mOpcode & 0b00000111));

    prefetchOpcode();
}

void CPU::opcode_ldr8imm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldr8imm8_M2;
}

void CPU::opcode_ldr8imm8_M2()
{
    writeR8((mOpcode & 0b00111000) >> 3, mDataZ);

    prefetchOpcode();
}

void CPU::opcode_ldr8ihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_ldr8ihl_M2;
}

void CPU::opcode_ldr8ihl_M2()
{
    writeR8((mOpcode & 0b00111000) >> 3, mDataZ);

    prefetchOpcode();
}

void CPU::opcode_ldihlr8_M1()
{
    writeByteAtHL(readR8(mOpcode & 0b00000111));

    mMCycleFunc = &CPU::opcode_ldihlr8_M2;
}

void CPU::opcode_ldihlr8_M2()
{
    prefetchOpcode();
}

void CPU::opcode_ldihlimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldihlimm8_M2;
}

void CPU::opcode_ldihlimm8_M2()
{
    writeByteAtHL(mDataZ);

    mMCycleFunc = &CPU::opcode_ldihlimm8_M3;
}

void CPU::opcode_ldihlimm8_M3()
{
    prefetchOpcode();
}

void CPU::opcode_ldair16mem_M1()
{
    mDataZ = mGameboy->readByte(readR16Mem((mOpcode & 0b00110000) >> 4));

    mMCycleFunc = &CPU::opcode_ldair16mem_M2;
}

void CPU::opcode_ldair16mem_M2()
{
    mRegA = mDataZ;

    prefetchOpcode();
}

void CPU::opcode_ldir16mema_M1()
{
    mGameboy->writeByte(readR16Mem((mOpcode & 0b00110000) >> 4), mRegA);

    mMCycleFunc = &CPU::opcode_ldir16mema_M2;
}

void CPU::opcode_ldir16mema_M2()
{
    prefetchOpcode();
}

void CPU::opcode_ldaiimm16_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldaiimm16_M2;
}

void CPU::opcode_ldaiimm16_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldaiimm16_M3;
}

void CPU::opcode_ldaiimm16_M3()
{
    mDataZ = mGameboy->readByte((mDataW << 8) + mDataZ);

    mMCycleFunc = &CPU::opcode_ldaiimm16_M4;
}

void CPU::opcode_ldaiimm16_M4()
{
    mRegA = mDataZ;

    prefetchOpcode();
}

void CPU::opcode_ldiimm16a_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldiimm16a_M2;
}

void CPU::opcode_ldiimm16a_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldiimm16a_M3;
}

void CPU::opcode_ldiimm16a_M3()
{
    mGameboy->writeByte((mDataW << 8) + mDataZ, mRegA);

    mMCycleFunc = &CPU::opcode_ldiimm16a_M4;
}

void CPU::opcode_ldiimm16a_M4()
{
    prefetchOpcode();
}

void CPU::opcode_ldhaic_M1()
{
    mDataZ = mGameboy->readByte(0xFF00 + mRegC);

    mMCycleFunc = &CPU::opcode_ldhaic_M2;
}

void CPU::opcode_ldhaic_M2()
{
    mRegA = mDataZ;

    prefetchOpcode();
}

void CPU::opcode_ldhica_M1()
{
    mGameboy->writeByte(0xFF00 + mRegC, mRegA);

    mMCycleFunc = &CPU::opcode_ldhica_M2;
}

void CPU::opcode_ldhica_M2()
{
    prefetchOpcode();
}

void CPU::opcode_ldhaiimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldhaiimm8_M2;
}

void CPU::opcode_ldhaiimm8_M2()
{
    mDataZ = mGameboy->readByte(0xFF00 + mDataZ);

    mMCycleFunc = &CPU::opcode_ldhaiimm8_M3;
}

void CPU::opcode_ldhaiimm8_M3()
{
    mRegA = mDataZ;

    prefetchOpcode();
}

void CPU::opcode_ldhiimm8a_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldhiimm8a_M2;
}

void CPU::opcode_ldhiimm8a_M2()
{
    mGameboy->writeByte(0xFF00 + mDataZ, mRegA);

    mMCycleFunc = &CPU::opcode_ldhiimm8a_M3;
}

void CPU::opcode_ldhiimm8a_M3()
{
    prefetchOpcode();
}

void CPU::opcode_ldr16imm16_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldr16imm16_M2;
}

void CPU::opcode_ldr16imm16_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldr16imm16_M3;
}

void CPU::opcode_ldr16imm16_M3()
{
    writeR16((mOpcode & 0b00110000) >> 4, (mDataW << 8) + mDataZ);

    prefetchOpcode();
}

void CPU::opcode_ldiimm16sp_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldiimm16sp_M2;
}

void CPU::opcode_ldiimm16sp_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldiimm16sp_M3;
}

void CPU::opcode_ldiimm16sp_M3()
{
    mGameboy->writeByte((mDataW << 8) + mDataZ, mSP & 0xFF);

    mMCycleFunc = &CPU::opcode_ldiimm16sp_M4;
}

void CPU::opcode_ldiimm16sp_M4()
{
    mGameboy->writeByte((mDataW << 8) + mDataZ + 1, mSP >> 8);

    mMCycleFunc = &CPU::opcode_ldiimm16sp_M5;
}

void CPU::opcode_ldiimm16sp_M5()
{
    prefetchOpcode();
}

void CPU::opcode_ldsphl_M1()
{
    mSP = (mRegH << 8) + mRegL;

    mMCycleFunc = &CPU::opcode_ldsphl_M2;
}

void CPU::opcode_ldsphl_M2()
{
    prefetchOpcode();
}

void CPU::opcode_pushr16stk_M1()
{
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_pushr16stk_M2;
}

void CPU::opcode_pushr16stk_M2()
{
    // LD [SP], HIGH(r16)  ; B, D or H
    mGameboy->writeByte(mSP, readR16StkHigh((mOpcode & 0b00110000) >> 4));
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_pushr16stk_M3;
}

void CPU::opcode_pushr16stk_M3()
{
    // LD [SP], LOW(r16)   ; C, E or L
    mGameboy->writeByte(mSP, readR16StkLow((mOpcode & 0b00110000) >> 4));

    mMCycleFunc = &CPU::opcode_pushr16stk_M4;
}

void CPU::opcode_pushr16stk_M4()
{
    prefetchOpcode();
}

void CPU::opcode_popr16stk_M1()
{
    // [SP]
    mDataZ = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_popr16stk_M2;
}

void CPU::opcode_popr16stk_M2()
{
    // [SP]
    mDataW = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_popr16stk_M3;
}

void CPU::opcode_popr16stk_M3()
{
    // LD LOW(r16), [SP]   ; C, E or L
    writeR16StkLow((mOpcode & 0b00110000) >> 4, mDataZ);
    // LD HIGH(r16), [SP]  ; B, D or H
    writeR16StkHigh((mOpcode & 0b00110000) >> 4, mDataW);

    prefetchOpcode();
}

void CPU::opcode_ldhlspimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_ldhlspimm8_M2;
}

void CPU::opcode_ldhlspimm8_M2()
{
    int8_t offset = mDataZ;
    uint16_t result = mSP + offset;
    mRegL = result & 0xFF;
    mRegF = 0;
    if (((mSP & 0xFF) + (mDataZ & 0xFF)) > 0xFF) // overflow from bit 7
        mRegF |= FLAG_CARRY;
    if (((mSP & 0xF) + (mDataZ & 0xF)) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    mMCycleFunc = &CPU::opcode_ldhlspimm8_M3;
}

void CPU::opcode_ldhlspimm8_M3()
{
    int8_t offset = mDataZ;
    uint16_t result = mSP + offset;
    mRegH = result >> 8;

    prefetchOpcode();
}

void CPU::opcode_addar8_M1()
{
    uint8_t orig = mRegA;
    uint8_t operand = readR8(mOpcode & 0b00000111);
    uint16_t result = mRegA + operand;
    mRegA = result & 0xFF;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    if (result > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand & 0xF)) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_addaihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_addaihl_M2;
}

void CPU::opcode_addaihl_M2()
{
    uint8_t orig = mRegA;
    uint8_t operand = mDataZ;
    uint16_t result = mRegA + operand;
    mRegA = result & 0xFF;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    if (result > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand & 0xF)) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_addaimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_addaimm8_M2;
}

void CPU::opcode_addaimm8_M2()
{
    uint8_t orig = mRegA;
    uint8_t operand = mDataZ;
    uint16_t result = mRegA + operand;
    mRegA = result & 0xFF;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    if (result > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand & 0xF)) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_adcar8_M1()
{
    uint16_t orig = mRegA;
    uint8_t operand1 = readR8(mOpcode & 0b00000111);
    uint8_t operand2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t operand = operand1 + operand2;
    uint16_t result = mRegA + operand;
    mRegA = result & 0xFF;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    if (result > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand1 & 0xF)) > 0xF ||
        ((orig & 0xF) + (operand1 & 0xF) + operand2) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_adcaihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_adcaihl_M2;
}

void CPU::opcode_adcaihl_M2()
{
    uint16_t orig = mRegA;
    uint8_t operand1 = mDataZ;
    uint8_t operand2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t operand = operand1 + operand2;
    uint16_t result = mRegA + operand;
    mRegA = result & 0xFF;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    if (result > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand1 & 0xF)) > 0xF ||
        ((orig & 0xF) + (operand1 & 0xF) + operand2) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_adcaimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_adcaimm8_M2;
}

void CPU::opcode_adcaimm8_M2()
{
    uint8_t orig = mRegA;
    uint8_t operand1 = mDataZ;
    uint8_t operand2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t operand = operand1 + operand2;
    uint16_t result = mRegA + operand;
    mRegA = result & 0xFF;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    if (result > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand1 & 0xF)) > 0xF ||
        ((orig & 0xF) + (operand1 & 0xF) + operand2) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_subar8_M1()
{
    uint8_t orig = mRegA;
    uint8_t sub = readR8(mOpcode & 0b00000111);
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;
    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_subaihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_subaihl_M2;
}

void CPU::opcode_subaihl_M2()
{
    uint8_t orig = mRegA;
    uint8_t sub = mDataZ;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;
    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_subaimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_subaimm8_M2;
}

void CPU::opcode_subaimm8_M2()
{
    uint8_t orig = mRegA;
    uint8_t sub = mDataZ;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;
    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_sbcar8_M1()
{
    uint16_t orig = mRegA;
    uint8_t sub1 = readR8(mOpcode & 0b00000111);
    uint8_t sub2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t sub = sub1 + sub2;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if (((sub1 & 0xF) > (orig & 0xF)) ||
        (sub2 > ((orig & 0xF) - (sub1 & 0xF)))) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;
    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_sbcaihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_sbcaihl_M2;
}

void CPU::opcode_sbcaihl_M2()
{
    uint16_t orig = mRegA;
    uint8_t sub1 = mDataZ;
    uint8_t sub2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t sub = sub1 + sub2;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if (((sub1 & 0xF) > (orig & 0xF)) ||
        (sub2 > ((orig & 0xF) - (sub1 & 0xF)))) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;
    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_sbcaimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_sbcaimm8_M2;
}

void CPU::opcode_sbcaimm8_M2()
{
    uint8_t orig = mRegA;
    uint8_t sub1 = mDataZ;
    uint8_t sub2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t sub = sub1 + sub2;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if (((sub1 & 0xF) > (orig & 0xF)) ||
        (sub2 > ((orig & 0xF) - (sub1 & 0xF)))) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;
    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_cpar8_M1()
{
    uint8_t orig = mRegA;
    uint8_t sub = readR8(mOpcode & 0b00000111);
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_cpaihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_cpaihl_M2;
}

void CPU::opcode_cpaihl_M2()
{
    uint8_t orig = mRegA;
    uint8_t sub = mDataZ;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_cpaimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_cpaimm8_M2;
}

void CPU::opcode_cpaimm8_M2()
{
    uint8_t orig = mRegA;
    uint8_t sub = mDataZ;
    uint8_t result = mRegA - sub;
    mRegF = FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (sub > mRegA)
        mRegF |= FLAG_CARRY;
    if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
        mRegF |= FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_incr8_M1()
{
    uint8_t orig = readR8((mOpcode & 0b00111000) >> 3);
    uint8_t result = orig + 1;
    mRegF &= FLAG_CARRY;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if ((orig & (1 << 4)) != (result & (1 << 4)))
        mRegF |= FLAG_HALFCARRY;

    writeR8((mOpcode & 0b00111000) >> 3, result);

    prefetchOpcode();
}

void CPU::opcode_incihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_incihl_M2;
}

void CPU::opcode_incihl_M2()
{
    uint8_t orig = mDataZ;
    uint8_t result = orig + 1;
    mRegF &= FLAG_CARRY;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if ((orig & (1 << 4)) != (result & (1 << 4)))
        mRegF |= FLAG_HALFCARRY;

    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_incihl_M3;
}

void CPU::opcode_incihl_M3()
{
    prefetchOpcode();
}

void CPU::opcode_decr8_M1()
{
    uint8_t orig = readR8((mOpcode & 0b00111000) >> 3);
    uint8_t sub = 1;
    uint8_t result = orig - sub;
    mRegF &= FLAG_CARRY;
    mRegF |= FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if ((orig & (1 << 4)) != (result & (1 << 4)))
        mRegF |= FLAG_HALFCARRY;

    writeR8((mOpcode & 0b00111000) >> 3, result);

    prefetchOpcode();
}

void CPU::opcode_decihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_decihl_M2;
}

void CPU::opcode_decihl_M2()
{
    uint8_t orig = mDataZ;
    uint8_t sub = 1;
    uint8_t result = orig - sub;
    mRegF &= FLAG_CARRY;
    mRegF |= FLAG_SUB;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if ((orig & (1 << 4)) != (result & (1 << 4)))
        mRegF |= FLAG_HALFCARRY;

    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_decihl_M3;
}

void CPU::opcode_decihl_M3()
{
    prefetchOpcode();
}

void CPU::opcode_andar8_M1()
{
    mRegA &= readR8(mOpcode & 0b00000111);
    mRegF = FLAG_HALFCARRY;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_andaihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_andaihl_M2;
}

void CPU::opcode_andaihl_M2()
{
    mRegA &= mDataZ;
    mRegF = FLAG_HALFCARRY;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_andaimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_andaimm8_M2;
}

void CPU::opcode_andaimm8_M2()
{
    mRegA &= mDataZ;
    mRegF = FLAG_HALFCARRY;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_orar8_M1()
{
    mRegA |= readR8(mOpcode & 0b00000111);
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_oraihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_oraihl_M2;
}

void CPU::opcode_oraihl_M2()
{
    mRegA |= mDataZ;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_oraimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_oraimm8_M2;
}

void CPU::opcode_oraimm8_M2()
{
    mRegA |= mDataZ;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_xorar8_M1()
{
    mRegA ^= readR8(mOpcode & 0b00000111);
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_xoraihl_M1()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_xoraihl_M2;
}

void CPU::opcode_xoraihl_M2()
{
    mRegA ^= mDataZ;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_xoraimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_xoraimm8_M2;
}

void CPU::opcode_xoraimm8_M2()
{
    mRegA ^= mDataZ;
    mRegF = 0;
    if (mRegA == 0)
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_ccf_M1()
{
    mRegF &= FLAG_ZERO | FLAG_CARRY;
    mRegF ^= FLAG_CARRY;

    prefetchOpcode();
}

void CPU::opcode_scf_M1()
{
    mRegF &= FLAG_ZERO;
    mRegF |= FLAG_CARRY;

    prefetchOpcode();
}

void CPU::opcode_daa_M1()
{
    uint8_t adjust = 0;
    bool setCarry = false;
    if (mRegF & FLAG_SUB)
    {
        if (mRegF & FLAG_HALFCARRY)
        {
            adjust += 0x6;
        }
        if (mRegF & FLAG_CARRY)
        {
            adjust += 0x60;
        }
        mRegA -= adjust;
    }
    else
    {
        if ((mRegF & FLAG_HALFCARRY) || ((mRegA & 0xF) > 0x9))
        {
            adjust += 0x6;
        }
        if ((mRegF & FLAG_CARRY) || (mRegA > 0x99))
        {
            adjust += 0x60;
            setCarry = true;
        }
        mRegA += adjust;
    }

    if (mRegA == 0)
        mRegF |= FLAG_ZERO;
    else
        mRegF &= ~FLAG_ZERO;
    mRegF &= ~FLAG_HALFCARRY;
    if (setCarry)
        mRegF |= FLAG_CARRY;
    // carry not reset to 0 in else case?

    prefetchOpcode();
}

void CPU::opcode_cpl_M1()
{
    mRegA = ~mRegA;
    mRegF |= FLAG_SUB | FLAG_HALFCARRY;

    prefetchOpcode();
}

void CPU::opcode_incr16_M1()
{
    writeR16((mOpcode & 0b00110000) >> 4, readR16((mOpcode & 0b00110000) >> 4) + 1);

    mMCycleFunc = &CPU::opcode_incr16_M2;
}

void CPU::opcode_incr16_M2()
{
    prefetchOpcode();
}

void CPU::opcode_decr16_M1()
{
    writeR16((mOpcode & 0b00110000) >> 4, readR16((mOpcode & 0b00110000) >> 4) - 1);

    mMCycleFunc = &CPU::opcode_decr16_M2;
}

void CPU::opcode_decr16_M2()
{
    prefetchOpcode();
}

void CPU::opcode_addhlr16_M1()
{
    uint16_t l = mRegL;
    uint16_t operand = (readR16((mOpcode & 0b00110000) >> 4)) & 0xFF;
    uint16_t orig = l;
    l += operand;
    mRegF &= FLAG_ZERO;
    if (l > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand & 0xF)) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;
    mRegL = l & 0xFF;

    mMCycleFunc = &CPU::opcode_addhlr16_M2;
}

void CPU::opcode_addhlr16_M2()
{
    uint16_t h = mRegH;
    uint16_t operand1 = (readR16((mOpcode & 0b00110000) >> 4)) >> 8;
    uint16_t operand2 = (mRegF & FLAG_CARRY) ? 1 : 0;
    uint16_t orig = h;
    h += operand1 + operand2;
    mRegF &= FLAG_ZERO;
    if (h > 0xFF)
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (operand1 & 0xF)) > 0xF ||
        ((orig & 0xF) + (operand1 & 0xF) + operand2) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;
    mRegH = h & 0xFF;

    prefetchOpcode();
}

void CPU::opcode_addspimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_addspimm8_M2;
}

void CPU::opcode_addspimm8_M2()
{
    uint16_t orig = mSP;
    int8_t imm8 = mDataZ;
    uint16_t result = mSP + imm8;
    mRegF = 0;
    if (((orig & 0xFF) + (imm8 & 0xFF)) > 0xFF) // overflow from bit 7
        mRegF |= FLAG_CARRY;
    if (((orig & 0xF) + (imm8 & 0xF)) > 0xF) // overflow from bit 3
        mRegF |= FLAG_HALFCARRY;

    mDataZ = result & 0xFF;
    mDataW = result >> 8; // technically this is set in the next cycle, let's hope it doesn't break

    mMCycleFunc = &CPU::opcode_addspimm8_M3;
}

void CPU::opcode_addspimm8_M3()
{
    mMCycleFunc = &CPU::opcode_addspimm8_M4;
}

void CPU::opcode_addspimm8_M4()
{
    mSP = (mDataW << 8) + mDataZ;

    prefetchOpcode();
}

void CPU::opcode_rlca_M1()
{
    mRegF = 0;
    if (mRegA & (1 << 7))
        mRegF |= FLAG_CARRY;
    mRegA = std::rotl(mRegA, 1);

    prefetchOpcode();
}

void CPU::opcode_rrca_M1()
{
    mRegF = 0;
    if (mRegA & 1)
        mRegF |= FLAG_CARRY;
    mRegA = std::rotr(mRegA, 1);

    prefetchOpcode();
}

void CPU::opcode_rla_M1()
{
    uint16_t origVal = (mRegF & FLAG_CARRY ? (1 << 8) : 0) + mRegA;
    mRegF = 0;

    uint16_t result = origVal << 1;
    if (result & (1 << 8))
        mRegF |= FLAG_CARRY;
    if (result & (1 << 9))
    {
        result &= 0b11111111;
        result |= 1;
    }

    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_rra_M1()
{
    uint16_t origVal = (mRegA << 1) + (mRegF & FLAG_CARRY ? 1 : 0);
    mRegF = 0;
    if (origVal & 0b10)
        mRegF |= FLAG_CARRY;

    uint16_t result = origVal >> 2;
    if (origVal & 1)
    {
        result |= (1 << 7);
    }

    mRegA = result;

    prefetchOpcode();
}

void CPU::opcode_rlcr8_M2()
{
    uint8_t origVal = readR8(mOpcode & 0b00000111);
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & (1 << 7))
        mRegF |= FLAG_CARRY;
    writeR8(mOpcode & 0b00000111, std::rotl(origVal, 1));

    prefetchOpcode();
}

void CPU::opcode_rlcihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_rlcihl_M3;
}

void CPU::opcode_rlcihl_M3()
{
    uint8_t origVal = mDataZ;
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & (1 << 7))
        mRegF |= FLAG_CARRY;
    writeByteAtHL(std::rotl(origVal, 1));

    mMCycleFunc = &CPU::opcode_rlcihl_M4;
}

void CPU::opcode_rlcihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_rrcr8_M2()
{
    uint8_t origVal = readR8(mOpcode & 0b00000111);
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & 1)
        mRegF |= FLAG_CARRY;
    writeR8(mOpcode & 0b00000111, std::rotr(origVal, 1));

    prefetchOpcode();
}

void CPU::opcode_rrcihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_rrcihl_M3;
}

void CPU::opcode_rrcihl_M3()
{
    uint8_t origVal = mDataZ;
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & 1)
        mRegF |= FLAG_CARRY;
    writeByteAtHL(std::rotr(origVal, 1));

    mMCycleFunc = &CPU::opcode_rrcihl_M4;
}

void CPU::opcode_rrcihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_rlr8_M2()
{
    uint16_t origVal = (mRegF & FLAG_CARRY ? (1 << 8) : 0) + readR8(mOpcode & 0b00000111);
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;

    uint16_t result = origVal << 1;
    if (result & (1 << 8))
        mRegF |= FLAG_CARRY;
    if (result & (1 << 9))
    {
        result &= 0b11111111;
        result |= 1;
    }

    writeR8(mOpcode & 0b00000111, result);

    prefetchOpcode();
}

void CPU::opcode_rlihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_rlihl_M3;
}

void CPU::opcode_rlihl_M3()
{
    uint16_t origVal = (mRegF & FLAG_CARRY ? (1 << 8) : 0) + mDataZ;
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;

    uint16_t result = origVal << 1;
    if (result & (1 << 8))
        mRegF |= FLAG_CARRY;
    if (result & (1 << 9))
    {
        result &= 0b11111111;
        result |= 1;
    }

    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_rlihl_M4;
}

void CPU::opcode_rlihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_rrr8_M2()
{
    uint16_t origVal = (readR8(mOpcode & 0b00000111) << 1) + (mRegF & FLAG_CARRY ? 1 : 0);
    mRegF = 0;
    if (origVal & 0b10)
        mRegF |= FLAG_CARRY;

    uint16_t result = origVal >> 2;
    if (origVal & 1)
    {
        result |= (1 << 7);
    }
    if (result == 0)
        mRegF |= FLAG_ZERO;

    writeR8(mOpcode & 0b00000111, result);

    prefetchOpcode();
}

void CPU::opcode_rrihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_rrihl_M3;
}

void CPU::opcode_rrihl_M3()
{
    uint16_t origVal = (mDataZ << 1) + (mRegF & FLAG_CARRY ? 1 : 0);
    mRegF = 0;
    if (origVal & 0b10)
        mRegF |= FLAG_CARRY;

    uint16_t result = origVal >> 2;
    if (origVal & 1)
    {
        result |= (1 << 7);
    }
    if (result == 0)
        mRegF |= FLAG_ZERO;

    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_rrihl_M4;
}

void CPU::opcode_rrihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_slar8_M2()
{
    uint8_t origVal = readR8(mOpcode & 0b00000111);
    uint8_t result = origVal << 1;
    mRegF = 0;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & (1 << 7))
        mRegF |= FLAG_CARRY;
    writeR8(mOpcode & 0b00000111, result);

    prefetchOpcode();
}

void CPU::opcode_slaihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_slaihl_M3;
}

void CPU::opcode_slaihl_M3()
{
    uint8_t origVal = mDataZ;
    uint8_t result = origVal << 1;
    mRegF = 0;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & (1 << 7))
        mRegF |= FLAG_CARRY;
    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_slaihl_M4;
}

void CPU::opcode_slaihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_srar8_M2()
{
    int8_t origVal = readR8(mOpcode & 0b00000111);
    int8_t result = origVal >> 1;
    mRegF = 0;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & 1)
        mRegF |= FLAG_CARRY;
    writeR8(mOpcode & 0b00000111, result);

    prefetchOpcode();
}

void CPU::opcode_sraihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_sraihl_M3;
}

void CPU::opcode_sraihl_M3()
{
    int8_t origVal = mDataZ;
    int8_t result = origVal >> 1;
    mRegF = 0;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & 1)
        mRegF |= FLAG_CARRY;
    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_sraihl_M4;
}

void CPU::opcode_sraihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_swapr8_M2()
{
    uint8_t origVal = readR8(mOpcode & 0b00000111);
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;
    uint8_t low = origVal & 0b1111;
    uint8_t high = origVal & 0b11110000;
    writeR8(mOpcode & 0b00000111, (low << 4) + (high >> 4));

    prefetchOpcode();
}

void CPU::opcode_swapihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_swapihl_M3;
}

void CPU::opcode_swapihl_M3()
{
    uint8_t origVal = mDataZ;
    mRegF = 0;
    if (origVal == 0)
        mRegF |= FLAG_ZERO;
    uint8_t low = origVal & 0b1111;
    uint8_t high = origVal & 0b11110000;
    writeByteAtHL((low << 4) + (high >> 4));

    mMCycleFunc = &CPU::opcode_swapihl_M4;
}

void CPU::opcode_swapihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_srlr8_M2()
{
    uint8_t origVal = readR8(mOpcode & 0b00000111);
    uint8_t result = origVal >> 1;
    mRegF = 0;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & 1)
        mRegF |= FLAG_CARRY;
    writeR8(mOpcode & 0b00000111, result);

    prefetchOpcode();
}

void CPU::opcode_srlihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_srlihl_M3;
}

void CPU::opcode_srlihl_M3()
{
    uint8_t origVal = mDataZ;
    uint8_t result = origVal >> 1;
    mRegF = 0;
    if (result == 0)
        mRegF |= FLAG_ZERO;
    if (origVal & 1)
        mRegF |= FLAG_CARRY;
    writeByteAtHL(result);

    mMCycleFunc = &CPU::opcode_srlihl_M4;
}

void CPU::opcode_srlihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_bitb3r8_M2()
{
    uint8_t b3 = (mOpcode & 0b00111000) >> 3;
    mRegF = FLAG_HALFCARRY | (mRegF & FLAG_CARRY);
    if (!(readR8(mOpcode & 0b00000111) & (1 << b3)))
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_bitb3ihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_bitb3ihl_M3;
}

void CPU::opcode_bitb3ihl_M3()
{
    uint8_t b3 = (mOpcode & 0b00111000) >> 3;
    mRegF = FLAG_HALFCARRY | (mRegF & FLAG_CARRY);
    if (!(mDataZ & (1 << b3)))
        mRegF |= FLAG_ZERO;

    prefetchOpcode();
}

void CPU::opcode_resb3r8_M2()
{
    uint8_t b3 = (mOpcode & 0b00111000) >> 3;
    writeR8(mOpcode & 0b00000111, readR8(mOpcode & 0b00000111) & ~(1 << b3));

    prefetchOpcode();
}

void CPU::opcode_resb3ihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_resb3ihl_M3;
}

void CPU::opcode_resb3ihl_M3()
{
    uint8_t b3 = (mOpcode & 0b00111000) >> 3;
    writeByteAtHL(mDataZ & ~(1 << b3));

    mMCycleFunc = &CPU::opcode_resb3ihl_M4;
}

void CPU::opcode_resb3ihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_setb3r8_M2()
{
    uint8_t b3 = (mOpcode & 0b00111000) >> 3;
    writeR8(mOpcode & 0b00000111, readR8(mOpcode & 0b00000111) | (1 << b3));

    prefetchOpcode();
}

void CPU::opcode_setb3ihl_M2()
{
    mDataZ = readByteAtHL();

    mMCycleFunc = &CPU::opcode_setb3ihl_M3;
}

void CPU::opcode_setb3ihl_M3()
{
    uint8_t b3 = (mOpcode & 0b00111000) >> 3;
    writeByteAtHL(mDataZ | (1 << b3));

    mMCycleFunc = &CPU::opcode_setb3ihl_M4;
}

void CPU::opcode_setb3ihl_M4()
{
    prefetchOpcode();
}

void CPU::opcode_jpimm16_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_jpimm16_M2;
}

void CPU::opcode_jpimm16_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_jpimm16_M3;
}

void CPU::opcode_jpimm16_M3()
{
    mPC = (mDataW << 8) + mDataZ;

    mMCycleFunc = &CPU::opcode_jpimm16_M4;
}

void CPU::opcode_jpimm16_M4()
{
    prefetchOpcode();
}

void CPU::opcode_jphl_M1()
{
    mPC = (mRegH << 8) + mRegL;

    prefetchOpcode();
}

void CPU::opcode_jrimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_jrimm8_M2;
}

void CPU::opcode_jrimm8_M2()
{
    int8_t offset = mDataZ;
    uint16_t result = mPC + offset;
    mDataZ = result & 0xFF;
    mDataW = result >> 8;

    mMCycleFunc = &CPU::opcode_jrimm8_M3;
}

void CPU::opcode_jrimm8_M3()
{
    mPC = (mDataW << 8) + mDataZ;
    prefetchOpcode();
}

void CPU::opcode_callimm16_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_callimm16_M2;
}

void CPU::opcode_callimm16_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_callimm16_M3;
}

void CPU::opcode_callimm16_M3()
{
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_callimm16_M4;
}

void CPU::opcode_callimm16_M4()
{
    // LD [SP], HIGH(mPC)
    mGameboy->writeByte(mSP, mPC >> 8);
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_callimm16_M5;
}

void CPU::opcode_callimm16_M5()
{
    // LD [SP], LOW(mPC)
    mGameboy->writeByte(mSP, mPC & 0xFF);
    mPC = (mDataW << 8) + mDataZ;

    mMCycleFunc = &CPU::opcode_callimm16_M6;
}

void CPU::opcode_callimm16_M6()
{
    prefetchOpcode();
}

void CPU::opcode_ret_M1()
{
    // LD LOW(pc), [SP]
    mDataZ = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_ret_M2;
}

void CPU::opcode_ret_M2()
{
    // LD HIGH(pc), [SP]
    mDataW = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_ret_M3;
}

void CPU::opcode_ret_M3()
{
    mPC = (mDataW << 8) + mDataZ;

    mMCycleFunc = &CPU::opcode_ret_M4;
}

void CPU::opcode_ret_M4()
{
    prefetchOpcode();
}

void CPU::opcode_reti_M1()
{
    // LD LOW(pc), [SP]
    mDataZ = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_reti_M2;
}

void CPU::opcode_reti_M2()
{
    // LD HIGH(pc), [SP]
    mDataW = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_reti_M3;
}

void CPU::opcode_reti_M3()
{
    mPC = (mDataW << 8) + mDataZ;
    // TODO set IME = 1

    mMCycleFunc = &CPU::opcode_reti_M4;
}

void CPU::opcode_reti_M4()
{
    prefetchOpcode();
}

void CPU::opcode_rsttgt3_M1()
{
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_rsttgt3_M2;
}

void CPU::opcode_rsttgt3_M2()
{
    // LD [SP], HIGH(mPC)
    mGameboy->writeByte(mSP, mPC >> 8);
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_rsttgt3_M3;
}

void CPU::opcode_rsttgt3_M3()
{
    // LD [SP], LOW(mPC)
    mGameboy->writeByte(mSP, mPC & 0xFF);

    mPC = mOpcode & 0b00111000;

    mMCycleFunc = &CPU::opcode_rsttgt3_M4;
}

void CPU::opcode_rsttgt3_M4()
{
    prefetchOpcode();
}

void CPU::opcode_di_M1()
{
    // TODO
    prefetchOpcode();
}

void CPU::opcode_ei_M1()
{
    // TODO
    prefetchOpcode();
}

void CPU::opcode_jpcondimm16_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_jpcondimm16_M2;
}

void CPU::opcode_jpcondimm16_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_jpcondimm16_M3;
}

void CPU::opcode_jpcondimm16_M3()
{
    if (checkCond((mOpcode & 0b00011000) >> 3))
    {
        mPC = (mDataW << 8) + mDataZ;
        mMCycleFunc = &CPU::opcode_jpcondimm16_M4;
    }
    else
    {
        prefetchOpcode();
    }
}

void CPU::opcode_jpcondimm16_M4()
{
    prefetchOpcode();
}

void CPU::opcode_jrcondimm8_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_jrcondimm8_M2;
}

void CPU::opcode_jrcondimm8_M2()
{
    if (checkCond((mOpcode & 0b00011000) >> 3))
    {
        int8_t offset = mDataZ;
        mPC += offset;
        mMCycleFunc = &CPU::opcode_jrcondimm8_M3;
    }
    else
    {
        prefetchOpcode();
    }
}

void CPU::opcode_jrcondimm8_M3()
{
    prefetchOpcode();
}

void CPU::opcode_callcondimm16_M1()
{
    mDataZ = readByteAtPC();
    mPC += 1;

    mMCycleFunc = &CPU::opcode_callcondimm16_M2;
}

void CPU::opcode_callcondimm16_M2()
{
    mDataW = readByteAtPC();
    mPC += 1;

    if (checkCond((mOpcode & 0b00011000) >> 3))
    {
        // push mPC + 3
        mMCycleFunc = &CPU::opcode_callcondimm16_M3;
    }
    else
    {
        mMCycleFunc = &CPU::opcode_callcondimm16_M6;
    }
}

void CPU::opcode_callcondimm16_M3()
{
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_callcondimm16_M4;
}

void CPU::opcode_callcondimm16_M4()
{
    // LD [SP], HIGH(mPC)
    mGameboy->writeByte(mSP, mPC >> 8);
    // DEC SP
    mSP--;

    mMCycleFunc = &CPU::opcode_callcondimm16_M5;
}

void CPU::opcode_callcondimm16_M5()
{
    // LD [SP], LOW(mPC)
    mGameboy->writeByte(mSP, mPC & 0xFF);

    mPC = (mDataW << 8) + mDataZ;

    mMCycleFunc = &CPU::opcode_callcondimm16_M6;
}

void CPU::opcode_callcondimm16_M6()
{
    prefetchOpcode();
}

void CPU::opcode_retcond_M1()
{
    if (checkCond((mOpcode & 0b00011000) >> 3))
    {
        // "pop pc"
        mMCycleFunc = &CPU::opcode_retcond_M2;
    }
    else
    {
        mMCycleFunc = &CPU::opcode_retcond_M5;
    }
}

void CPU::opcode_retcond_M2()
{
    uint8_t low, high;
    // LD LOW(pc), [SP]
    mDataZ = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_retcond_M3;
}

void CPU::opcode_retcond_M3()
{
    // LD HIGH(pc), [SP]
    mDataW = mGameboy->readByte(mSP);
    // INC SP
    mSP++;

    mMCycleFunc = &CPU::opcode_retcond_M4;
}

void CPU::opcode_retcond_M4()
{
    mPC = (mDataW << 8) + mDataZ;

    mMCycleFunc = &CPU::opcode_retcond_M5;
}

void CPU::opcode_retcond_M5()
{
    prefetchOpcode();
}

void CPU::opcode_prefix_cb_M1()
{
    mOpcode = readByteAtPC();
    mPC++;
    mMCycleFunc = &CPU::decodeExecuteOpcodeCB;
}

void CPU::prefetchOpcode()
{
    mOpcode = readByteAtPC();
    mPC++;
    mMCycleFunc = &CPU::decodeExecuteOpcode;
}

void CPU::decodeExecuteOpcodeCB()
{
    auto opcode = mOpcode;
    if ((opcode & 0b11111000) == 0b00000000)
    {
        // rlc r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_rlcihl_M2();
        else
            opcode_rlcr8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00001000)
    {
        // rrc r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_rrcihl_M2();
        else
            opcode_rrcr8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00010000)
    {
        // rl r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_rlihl_M2();
        else
            opcode_rlr8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00011000)
    {
        // rr r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_rrihl_M2();
        else
            opcode_rrr8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00100000)
    {
        // sla r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_slaihl_M2();
        else
            opcode_slar8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00101000)
    {
        // sra r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_sraihl_M2();
        else
            opcode_srar8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00110000)
    {
        // swap r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_swapihl_M2();
        else
            opcode_swapr8_M2();
    }
    else if ((opcode & 0b11111000) == 0b00111000)
    {
        // srl r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_srlihl_M2();
        else
            opcode_srlr8_M2();
    }
    else if ((opcode & 0b11000000) == 0b01000000)
    {
        // bit b3, r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_bitb3ihl_M2();
        else
            opcode_bitb3r8_M2();
    }
    else if ((opcode & 0b11000000) == 0b10000000)
    {
        // res b3, r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_resb3ihl_M2();
        else
            opcode_resb3r8_M2();
    }
    else if ((opcode & 0b11000000) == 0b11000000)
    {
        // set b3, r8
        if ((opcode & 0b00000111) == HL_BITMASK)
            opcode_setb3ihl_M2();
        else
            opcode_setb3r8_M2();
    }
    else
    {
        throw std::runtime_error("Invalid opcode");
    }
}

void CPU::decodeExecuteOpcode()
{
    // Decode and execute
    auto opcode = mOpcode;
    if (opcode == 0xCB)
    {
        // Prefix
        opcode_prefix_cb_M1();
    }
    else if (opcode == 0xD3 || opcode == 0xDB || opcode == 0xDD || opcode == 0xE3 || opcode == 0xE4 || opcode == 0xEB || opcode == 0xEC || opcode == 0xED || opcode == 0xF4 || opcode == 0xFC || opcode == 0xFD)
    {
        throw std::runtime_error("Invalid opcode");
    }
    else if (opcode == 0)
    {
        // nop
        opcode_nop_M1();
    }
    else if ((opcode & 0b11001111) == 0b00000001)
    {
        // ld r16, imm16
        opcode_ldr16imm16_M1();
    }
    else if ((opcode & 0b11001111) == 0b00000010)
    {
        // ld [r16mem], a
        opcode_ldir16mema_M1();
    }
    else if ((opcode & 0b11001111) == 0b00001010)
    {
        // ld a, [r16mem]
        opcode_ldair16mem_M1();
    }
    else if (opcode == 0b00001000)
    {
        // ld [imm16], sp
        opcode_ldiimm16sp_M1();
    }
    else if ((opcode & 0b11001111) == 0b00000011)
    {
        // inc r16
        opcode_incr16_M1();
    }
    else if ((opcode & 0b11001111) == 0b00001011)
    {
        // dec r16
        opcode_decr16_M1();
    }
    else if ((opcode & 0b11001111) == 0b00001001)
    {
        // add hl, r16
        opcode_addhlr16_M1();
    }
    else if ((opcode & 0b11000111) == 0b00000100)
    {
        // inc r8
        if (((mOpcode & 0b00111000) >> 3) == HL_BITMASK)
            opcode_incihl_M1();
        else
            opcode_incr8_M1();
    }
    else if ((opcode & 0b11000111) == 0b00000101)
    {
        // dec r8
        if (((mOpcode & 0b00111000) >> 3) == HL_BITMASK)
            opcode_decihl_M1();
        else
            opcode_decr8_M1();
    }
    else if ((opcode & 0b11000111) == 0b00000110)
    {
        // ld r8, imm8
        if (((mOpcode & 0b00111000) >> 3) == HL_BITMASK)
            opcode_ldihlimm8_M1();
        else
            opcode_ldr8imm8_M1();
    }
    else if (opcode == 0b00000111)
    {
        // rlca
        opcode_rlca_M1();
    }
    else if (opcode == 0b00001111)
    {
        // rrca
        opcode_rrca_M1();
    }
    else if (opcode == 0b00010111)
    {
        // rla
        opcode_rla_M1();
    }
    else if (opcode == 0b00011111)
    {
        // rra
        opcode_rra_M1();
    }
    else if (opcode == 0b00100111)
    {
        // daa
        opcode_daa_M1();
    }
    else if (opcode == 0b00101111)
    {
        // cpl
        opcode_cpl_M1();
    }
    else if (opcode == 0b00110111)
    {
        // scf
        opcode_scf_M1();
    }
    else if (opcode == 0b00111111)
    {
        // ccf
        opcode_ccf_M1();
    }
    else if (opcode == 0b00011000)
    {
        // jr imm8
        opcode_jrimm8_M1();
    }
    else if ((opcode & 0b11100111) == 0b00100000)
    {
        // jr cond, imm8
        opcode_jrcondimm8_M1();
    }
    else if (opcode == 0b00010000)
    {
        // stop
        // TODO
        prefetchOpcode();
    }
    else if (opcode == 0b01110110)
    {
        // halt (ld [hl], [hl])
        // TODO
        prefetchOpcode();
    }
    else if ((opcode & 0b11000000) == 0b01000000)
    {
        // ld r8, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_ldr8ihl_M1();
        else if ((mOpcode & 0b00111000) >> 3 == HL_BITMASK)
            opcode_ldihlr8_M1();
        else
            opcode_ldr8r8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10000000)
    {
        // add a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_addaihl_M1();
        else
            opcode_addar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10001000)
    {
        // adc a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_adcaihl_M1();
        else
            opcode_adcar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10010000)
    {
        // sub a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_subaihl_M1();
        else
            opcode_subar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10011000)
    {
        // sbc a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_sbcaihl_M1();
        else
            opcode_sbcar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10100000)
    {
        // and a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_andaihl_M1();
        else
            opcode_andar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10101000)
    {
        // xor a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_xoraihl_M1();
        else
            opcode_xorar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10110000)
    {
        // or a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_oraihl_M1();
        else
            opcode_orar8_M1();
    }
    else if ((opcode & 0b11111000) == 0b10111000)
    {
        // cp a, r8
        if ((mOpcode & 0b00000111) == HL_BITMASK)
            opcode_cpaihl_M1();
        else
            opcode_cpar8_M1();
    }
    else if (opcode == 0b11000110)
    {
        // add a, imm8
        opcode_addaimm8_M1();
    }
    else if (opcode == 0b11001110)
    {
        // adc a, imm8
        opcode_adcaimm8_M1();
    }
    else if (opcode == 0b11010110)
    {
        // sub a, imm8
        opcode_subaimm8_M1();
    }
    else if (opcode == 0b11011110)
    {
        // sbc a, imm8
        opcode_sbcaimm8_M1();
    }
    else if (opcode == 0b11100110)
    {
        // and a, imm8
        opcode_andaimm8_M1();
    }
    else if (opcode == 0b11101110)
    {
        // xor a, imm8
        opcode_xoraimm8_M1();
    }
    else if (opcode == 0b11110110)
    {
        // or a, imm8
        opcode_oraimm8_M1();
    }
    else if (opcode == 0b11111110)
    {
        // cp a, imm8
        opcode_cpaimm8_M1();
    }
    else if ((opcode & 0b11100111) == 0b11000000)
    {
        // ret cond
        opcode_retcond_M1();
    }
    else if (opcode == 0b11001001)
    {
        // ret
        // "pop pc"
        opcode_ret_M1();
    }
    else if (opcode == 0b11011001)
    {
        // reti
        // Equivalent to executing EI then RET, meaning that IME is set right after this instruction
        opcode_reti_M1();
    }
    else if ((opcode & 0b11100111) == 0b11000010)
    {
        // jp cond, imm16
        opcode_jpcondimm16_M1();
    }
    else if (opcode == 0b11000011)
    {
        // jp imm16
        opcode_jpimm16_M1();
    }
    else if (opcode == 0b11101001)
    {
        // jp hl
        opcode_jphl_M1();
    }
    else if ((opcode & 0b11100111) == 0b11000100)
    {
        // call cond, imm16
        opcode_callcondimm16_M1();
    }
    else if (opcode == 0b11001101)
    {
        // call imm16
        // push mPC + 3
        opcode_callimm16_M1();
    }
    else if ((opcode & 0b11000111) == 0b11000111)
    {
        // rst tgt3
        // push mPC + 1
        opcode_rsttgt3_M1();
    }
    else if ((opcode & 0b11001111) == 0b11000001)
    {
        // pop r16stk
        opcode_popr16stk_M1();
    }
    else if ((opcode & 0b11001111) == 0b11000101)
    {
        // push r16stk
        opcode_pushr16stk_M1();
    }
    else if (opcode == 0b11100010)
    {
        // ldh [c], a
        opcode_ldhica_M1();
    }
    else if (opcode == 0b11100000)
    {
        // ldh [imm8], a
        opcode_ldhiimm8a_M1();
    }
    else if (opcode == 0b11101010)
    {
        // ld [imm16], a
        opcode_ldiimm16a_M1();
    }
    else if (opcode == 0b11110010)
    {
        // ldh a, [c]
        opcode_ldhaic_M1();
    }
    else if (opcode == 0b11110000)
    {
        // ldh a, [imm8]
        opcode_ldhaiimm8_M1();
    }
    else if (opcode == 0b11111010)
    {
        // ld a, [imm16]
        opcode_ldaiimm16_M1();
    }
    else if (opcode == 0b11101000)
    {
        // add sp, imm8
        opcode_addspimm8_M1();
    }
    else if (opcode == 0b11111000)
    {
        // ld hl, sp + imm8
        opcode_ldhlspimm8_M1();
    }
    else if (opcode == 0b11111001)
    {
        // ld sp, hl
        opcode_ldsphl_M1();
    }
    else if (opcode == 0b11110011)
    {
        // di
        opcode_di_M1();
    }
    else if (opcode == 0b11111011)
    {
        // ei
        opcode_ei_M1();
    }
    else
    {
        throw std::runtime_error("Invalid opcode");
    }
}
