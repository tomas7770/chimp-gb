#include "CPU.h"
#include "Gameboy.h"

uint16_t CPU::getImm16() const
{
    // Little-endian
    return (mGameboy->readByte(mPC - 1 + 2) << 8) + mGameboy->readByte(mPC - 1 + 1);
}

uint8_t CPU::getImm8() const
{
    return mGameboy->readByte(mPC - 1 + 1);
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
    case 6:
        return mGameboy->readByte((mRegH << 8) + mRegL);
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
    case 6:
        mGameboy->writeByte((mRegH << 8) + mRegL, value);
        break;
    case 7:
        mRegA = value;
        break;

    default:
        throw std::runtime_error("Invalid bitmask provided to r8 instruction parameter");
    }
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
        mRegF = value;
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

void CPU::fetchDecodeExecuteOpcode()
{
    auto opcode = mGameboy->readByte(mPC - 1);
    if (opcode == 0xCB)
    {
        // Prefix
        mPC++;
        opcode = mGameboy->readByte(mPC - 1);
        if ((opcode & 0b11111000) == 0b00000000)
        {
            // rlc r8
            uint8_t origVal = readR8(opcode & 0b00000111);
            mRegF = 0;
            if (origVal == 0)
                mRegF |= FLAG_ZERO;
            if (origVal & (1 << 7))
                mRegF |= FLAG_CARRY;
            writeR8(opcode & 0b00000111, std::rotl(origVal, 1));
        }
        else if ((opcode & 0b11111000) == 0b00001000)
        {
            // rrc r8
            uint8_t origVal = readR8(opcode & 0b00000111);
            mRegF = 0;
            if (origVal == 0)
                mRegF |= FLAG_ZERO;
            if (origVal & 1)
                mRegF |= FLAG_CARRY;
            writeR8(opcode & 0b00000111, std::rotr(origVal, 1));
        }
        else if ((opcode & 0b11111000) == 0b00010000)
        {
            // rl r8
            uint16_t origVal = (mRegF & FLAG_CARRY ? (1 << 8) : 0) + readR8(opcode & 0b00000111);
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

            writeR8(opcode & 0b00000111, result);
        }
        else if ((opcode & 0b11111000) == 0b00011000)
        {
            // rr r8
            uint16_t origVal = (readR8(opcode & 0b00000111) << 1) + (mRegF & FLAG_CARRY ? 1 : 0);
            mRegF = 0;
            if (origVal == 0)
                mRegF |= FLAG_ZERO;
            if (origVal & 0b10)
                mRegF |= FLAG_CARRY;

            uint16_t result = origVal >> 2;
            if (origVal & 1)
            {
                result |= (1 << 7);
            }

            writeR8(opcode & 0b00000111, result);
        }
        else if ((opcode & 0b11111000) == 0b00100000)
        {
            // sla r8
            uint8_t origVal = readR8(opcode & 0b00000111);
            uint8_t result = origVal << 1;
            mRegF = 0;
            if (result == 0)
                mRegF |= FLAG_ZERO;
            if (origVal & (1 << 7))
                mRegF |= FLAG_CARRY;
            writeR8(opcode & 0b00000111, result);
        }
        else if ((opcode & 0b11111000) == 0b00101000)
        {
            // sra r8
            int8_t origVal = readR8(opcode & 0b00000111);
            int8_t result = origVal >> 1;
            mRegF = 0;
            if (result == 0)
                mRegF |= FLAG_ZERO;
            if (origVal & 1)
                mRegF |= FLAG_CARRY;
            writeR8(opcode & 0b00000111, result);
        }
        else if ((opcode & 0b11111000) == 0b00110000)
        {
            // swap r8
            uint8_t origVal = readR8(opcode & 0b00000111);
            mRegF = 0;
            if (origVal == 0)
                mRegF |= FLAG_ZERO;
            uint8_t low = origVal & 0b1111;
            uint8_t high = origVal & 0b11110000;
            writeR8(opcode & 0b00000111, (low << 4) + (high >> 4));
        }
        else if ((opcode & 0b11111000) == 0b00111000)
        {
            // srl r8
            uint8_t origVal = readR8(opcode & 0b00000111);
            uint8_t result = origVal >> 1;
            mRegF = 0;
            if (result == 0)
                mRegF |= FLAG_ZERO;
            if (origVal & 1)
                mRegF |= FLAG_CARRY;
            writeR8(opcode & 0b00000111, result);
        }
        else if ((opcode & 0b11000000) == 0b01000000)
        {
            // bit b3, r8
            uint8_t b3 = (opcode & 0b00111000) >> 3;
            mRegF = FLAG_HALFCARRY | (mRegF & FLAG_CARRY);
            if (!(readR8(opcode & 0b00000111) & (1 << b3)))
                mRegF |= FLAG_ZERO;
        }
        else if ((opcode & 0b11000000) == 0b10000000)
        {
            // res b3, r8
            uint8_t b3 = (opcode & 0b00111000) >> 3;
            writeR8(opcode & 0b00000111, readR8(opcode & 0b00000111) & !(1 << b3));
        }
        else if ((opcode & 0b11000000) == 0b11000000)
        {
            // set b3, r8
            uint8_t b3 = (opcode & 0b00111000) >> 3;
            writeR8(opcode & 0b00000111, readR8(opcode & 0b00000111) | (1 << b3));
        }
        else
        {
            throw std::runtime_error("Invalid opcode");
        }
    }
    else if (opcode == 0xD3 || opcode == 0xDB || opcode == 0xDD || opcode == 0xE3 || opcode == 0xE4 || opcode == 0xEB || opcode == 0xEC || opcode == 0xED || opcode == 0xF4 || opcode == 0xFC || opcode == 0xFD)
    {
        throw std::runtime_error("Invalid opcode");
    }
    else if (opcode == 0)
    {
        // nop
    }
    else if ((opcode & 0b11001111) == 0b00000001)
    {
        // ld r16, imm16
        writeR16((opcode & 0b00110000) >> 4, getImm16());
        mPC += 2;
    }
    else if ((opcode & 0b11001111) == 0b00000010)
    {
        // ld [r16mem], a
        mGameboy->writeByte(readR16Mem((opcode & 0b00110000) >> 4), mRegA);
    }
    else if ((opcode & 0b11001111) == 0b00001010)
    {
        // ld a, [r16mem]
        mRegA = mGameboy->readByte(readR16Mem((opcode & 0b00110000) >> 4));
    }
    else if (opcode == 0b00001000)
    {
        // ld [imm16], sp
        mGameboy->writeWord(getImm16(), mSP);
        mPC += 2;
    }
    else if ((opcode & 0b11001111) == 0b00000011)
    {
        // inc r16
        writeR16((opcode & 0b00110000) >> 4, readR16((opcode & 0b00110000) >> 4) + 1);
    }
    else if ((opcode & 0b11001111) == 0b00001011)
    {
        // dec r16
        writeR16((opcode & 0b00110000) >> 4, readR16((opcode & 0b00110000) >> 4) - 1);
    }
    else if ((opcode & 0b11001111) == 0b00001001)
    {
        // add hl, r16
        uint32_t hl = (mRegH << 8) + mRegL;
        uint32_t orig = hl;
        uint32_t operand = readR16((opcode & 0b00110000) >> 4);
        hl += operand;
        mRegF &= FLAG_ZERO;
        if (hl > 0xFFFF)
            mRegF |= FLAG_CARRY;
        if (((orig & 0xFFF) + (operand & 0xFFF)) > 0xFFF) // overflow from bit 11
            mRegF |= FLAG_HALFCARRY;
        mRegH = hl >> 8;
        mRegL = hl & 0xFF;
    }
    else if ((opcode & 0b11000111) == 0b00000100)
    {
        // inc r8
        uint8_t orig = readR8((opcode & 0b00111000) >> 3);
        uint8_t result = orig + 1;
        mRegF &= FLAG_CARRY;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;

        writeR8((opcode & 0b00111000) >> 3, result);
    }
    else if ((opcode & 0b11000111) == 0b00000101)
    {
        // dec r8
        uint8_t orig = readR8((opcode & 0b00111000) >> 3);
        uint8_t sub = 1;
        uint8_t result = orig - sub;
        mRegF &= FLAG_CARRY;
        mRegF |= FLAG_SUB;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;

        writeR8((opcode & 0b00111000) >> 3, result);
    }
    else if ((opcode & 0b11000111) == 0b00000110)
    {
        // ld r8, imm8
        writeR8((opcode & 0b00111000) >> 3, getImm8());
        mPC += 1;
    }
    else if (opcode == 0b00000111)
    {
        // rlca
        mRegF = 0;
        if (mRegA & (1 << 7))
            mRegF |= FLAG_CARRY;
        mRegA = std::rotl(mRegA, 1);
    }
    else if (opcode == 0b00001111)
    {
        // rrca
        mRegF = 0;
        if (mRegA & 1)
            mRegF |= FLAG_CARRY;
        mRegA = std::rotr(mRegA, 1);
    }
    else if (opcode == 0b00010111)
    {
        // rla
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
    }
    else if (opcode == 0b00011111)
    {
        // rra
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
    }
    else if (opcode == 0b00100111)
    {
        // daa
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
    }
    else if (opcode == 0b00101111)
    {
        // cpl
        mRegA = ~mRegA;
        mRegF |= FLAG_SUB | FLAG_HALFCARRY;
    }
    else if (opcode == 0b00110111)
    {
        // scf
        mRegF &= FLAG_ZERO;
        mRegF |= FLAG_CARRY;
    }
    else if (opcode == 0b00111111)
    {
        // ccf
        mRegF &= FLAG_ZERO | FLAG_CARRY;
        mRegF ^= FLAG_CARRY;
    }
    else if (opcode == 0b00011000)
    {
        // jr imm8
        int8_t offset = getImm8();
        mPC += 1 + offset;
    }
    else if ((opcode & 0b11100111) == 0b00100000)
    {
        // jr cond, imm8
        if (checkCond((opcode & 0b00011000) >> 3))
        {
            int8_t offset = getImm8();
            mPC += offset;
        }
        mPC += 1;
    }
    else if (opcode == 0b00010000)
    {
        // stop
        // TODO
    }
    else if (opcode == 0b01110110)
    {
        // halt (ld [hl], [hl])
        // TODO
    }
    else if ((opcode & 0b11000000) == 0b01000000)
    {
        // ld r8, r8
        writeR8((opcode & 0b00111000) >> 3, readR8(opcode & 0b00000111));
    }
    else if ((opcode & 0b11111000) == 0b10000000)
    {
        // add a, r8
        uint8_t orig = mRegA;
        uint8_t operand = readR8(opcode & 0b00000111);
        uint16_t result = mRegA + operand;
        mRegA = result & 0xFF;
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
        if (result > 0xFF)
            mRegF |= FLAG_CARRY;
        if (((orig & 0xF) + (operand & 0xF)) > 0xF) // overflow from bit 3
            mRegF |= FLAG_HALFCARRY;
    }
    else if ((opcode & 0b11111000) == 0b10001000)
    {
        // adc a, r8
        uint16_t orig = mRegA;
        uint8_t operand1 = readR8(opcode & 0b00000111);
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
    }
    else if ((opcode & 0b11111000) == 0b10010000)
    {
        // sub a, r8
        uint8_t orig = mRegA;
        uint8_t sub = readR8(opcode & 0b00000111);
        uint8_t result = mRegA - sub;
        mRegF = FLAG_SUB;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if (sub > mRegA)
            mRegF |= FLAG_CARRY;
        if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
            mRegF |= FLAG_HALFCARRY;
        mRegA = result;
    }
    else if ((opcode & 0b11111000) == 0b10011000)
    {
        // sbc a, r8
        uint16_t orig = mRegA;
        uint8_t sub1 = readR8(opcode & 0b00000111);
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
    }
    else if ((opcode & 0b11111000) == 0b10100000)
    {
        // and a, r8
        mRegA &= readR8(opcode & 0b00000111);
        mRegF = FLAG_HALFCARRY;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
    }
    else if ((opcode & 0b11111000) == 0b10101000)
    {
        // xor a, r8
        mRegA ^= readR8(opcode & 0b00000111);
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
    }
    else if ((opcode & 0b11111000) == 0b10110000)
    {
        // or a, r8
        mRegA |= readR8(opcode & 0b00000111);
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
    }
    else if ((opcode & 0b11111000) == 0b10111000)
    {
        // cp a, r8
        uint8_t orig = mRegA;
        uint8_t sub = readR8(opcode & 0b00000111);
        uint8_t result = mRegA - sub;
        mRegF = FLAG_SUB;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if (sub > mRegA)
            mRegF |= FLAG_CARRY;
        if ((sub & 0xF) > (orig & 0xF)) // borrow from bit 4
            mRegF |= FLAG_HALFCARRY;
    }
    else if (opcode == 0b11000110)
    {
        // add a, imm8
        uint8_t orig = mRegA;
        uint8_t operand = getImm8();
        uint16_t result = mRegA + operand;
        mRegA = result & 0xFF;
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
        if (result > 0xFF)
            mRegF |= FLAG_CARRY;
        if (((orig & 0xF) + (operand & 0xF)) > 0xF) // overflow from bit 3
            mRegF |= FLAG_HALFCARRY;
        mPC += 1;
    }
    else if (opcode == 0b11001110)
    {
        // adc a, imm8
        uint8_t orig = mRegA;
        uint16_t result = mRegA + getImm8() + (mRegF & FLAG_CARRY ? 1 : 0);
        mRegA = result & 0xFF;
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
        if (result > 0xFF)
            mRegF |= FLAG_CARRY;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;
        mPC += 1;
    }
    else if (opcode == 0b11010110)
    {
        // sub a, imm8
        uint8_t orig = mRegA;
        uint8_t sub = getImm8();
        uint8_t result = mRegA - sub;
        mRegF = FLAG_SUB;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if (sub > mRegA)
            mRegF |= FLAG_CARRY;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;
        mRegA = result;
        mPC += 1;
    }
    else if (opcode == 0b11011110)
    {
        // sbc a, imm8
        uint8_t orig = mRegA;
        uint8_t sub = getImm8() + (mRegF & FLAG_CARRY ? 1 : 0);
        uint8_t result = mRegA - sub;
        mRegF = FLAG_SUB;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if (sub > mRegA)
            mRegF |= FLAG_CARRY;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;
        mPC += 1;
    }
    else if (opcode == 0b11100110)
    {
        // and a, imm8
        mRegA &= getImm8();
        mRegF = FLAG_HALFCARRY;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
        mPC += 1;
    }
    else if (opcode == 0b11101110)
    {
        // xor a, imm8
        mRegA ^= getImm8();
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
        mPC += 1;
    }
    else if (opcode == 0b11110110)
    {
        // or a, imm8
        mRegA |= getImm8();
        mRegF = 0;
        if (mRegA == 0)
            mRegF |= FLAG_ZERO;
        mPC += 1;
    }
    else if (opcode == 0b11111110)
    {
        // cp a, imm8
        uint8_t orig = mRegA;
        uint8_t sub = getImm8();
        uint8_t result = mRegA - sub;
        mRegF = FLAG_SUB;
        if (result == 0)
            mRegF |= FLAG_ZERO;
        if (sub > mRegA)
            mRegF |= FLAG_CARRY;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;
        mPC += 1;
    }
    else if ((opcode & 0b11100111) == 0b11000000)
    {
        // ret cond
        if (checkCond((opcode & 0b00011000) >> 3))
        {
            // "pop pc"
            uint8_t low, high;
            // LD LOW(pc), [SP]
            low = mGameboy->readByte(mSP);
            // INC SP
            mSP++;
            // LD HIGH(pc), [SP]
            high = mGameboy->readByte(mSP);
            // INC SP
            mSP++;

            mPC = (high << 8) + low;
        }
    }
    else if (opcode == 0b11001001)
    {
        // ret
        // "pop pc"
        uint8_t low, high;
        // LD LOW(pc), [SP]
        low = mGameboy->readByte(mSP);
        // INC SP
        mSP++;
        // LD HIGH(pc), [SP]
        high = mGameboy->readByte(mSP);
        // INC SP
        mSP++;

        mPC = (high << 8) + low;
    }
    else if (opcode == 0b11011001)
    {
        // reti
        // Equivalent to executing EI then RET, meaning that IME is set right after this instruction
        // TODO EI part

        // Same as ret
        // "pop pc"
        uint8_t low, high;
        // LD LOW(pc), [SP]
        low = mGameboy->readByte(mSP);
        // INC SP
        mSP++;
        // LD HIGH(pc), [SP]
        high = mGameboy->readByte(mSP);
        // INC SP
        mSP++;

        mPC = (high << 8 + low);
    }
    else if ((opcode & 0b11100111) == 0b11000010)
    {
        // jp cond, imm16
        if (checkCond((opcode & 0b00011000) >> 3))
        {
            mPC = getImm16();
        }
        else
        {
            mPC += 2;
        }
    }
    else if (opcode == 0b11000011)
    {
        // jp imm16
        mPC = getImm16();
    }
    else if (opcode == 0b11101001)
    {
        // jp hl
        mPC = mRegH << 8 + mRegL;
    }
    else if ((opcode & 0b11100111) == 0b11000100)
    {
        // call cond, imm16
        if (checkCond((opcode & 0b00011000) >> 3))
        {
            // push mPC + 3
            // DEC SP
            mSP--;
            // LD [SP], HIGH(mPC + 3)
            mGameboy->writeByte(mSP, (mPC + 3) >> 8);
            // DEC SP
            mSP--;
            // LD [SP], LOW(mPC + 3)
            mGameboy->writeByte(mSP, (mPC + 3) & 0xFF);

            mPC = getImm16();
        }
        else
        {
            mPC += 2;
        }
    }
    else if (opcode == 0b11001101)
    {
        // call imm16

        // push mPC + 3
        // DEC SP
        mSP--;
        // LD [SP], HIGH(mPC + 3)
        mGameboy->writeByte(mSP, (mPC + 3) >> 8);
        // DEC SP
        mSP--;
        // LD [SP], LOW(mPC + 3)
        mGameboy->writeByte(mSP, (mPC + 3) & 0xFF);

        mPC = getImm16();
    }
    else if ((opcode & 0b11000111) == 0b11000111)
    {
        // rst tgt3

        // push mPC + 1
        // DEC SP
        mSP--;
        // LD [SP], HIGH(mPC + 1)
        mGameboy->writeByte(mSP, (mPC + 1) >> 8);
        // DEC SP
        mSP--;
        // LD [SP], LOW(mPC + 1)
        mGameboy->writeByte(mSP, (mPC + 1) & 0xFF);

        mPC = ((opcode & 0b00111000) >> 3);
    }
    else if ((opcode & 0b11001111) == 0b11000001)
    {
        // pop r16stk
        // LD LOW(r16), [SP]   ; C, E or L
        writeR16StkLow((opcode & 0b00110000) >> 4, mGameboy->readByte(mSP));
        // INC SP
        mSP++;
        // LD HIGH(r16), [SP]  ; B, D or H
        writeR16StkHigh((opcode & 0b00110000) >> 4, mGameboy->readByte(mSP));
        // INC SP
        mSP++;
    }
    else if ((opcode & 0b11001111) == 0b11000101)
    {
        // push r16stk
        // DEC SP
        mSP--;
        // LD [SP], HIGH(r16)  ; B, D or H
        mGameboy->writeByte(mSP, readR16StkHigh((opcode & 0b00110000) >> 4));
        // DEC SP
        mSP--;
        // LD [SP], LOW(r16)   ; C, E or L
        mGameboy->writeByte(mSP, readR16StkLow((opcode & 0b00110000) >> 4));
    }
    else if (opcode == 0b11100010)
    {
        // ldh [c], a
        mGameboy->writeByte(0xFF00 + mRegC, mRegA);
    }
    else if (opcode == 0b11100000)
    {
        // ldh [imm8], a
        mGameboy->writeByte(0xFF00 + getImm8(), mRegA);
        mPC += 1;
    }
    else if (opcode == 0b11101010)
    {
        // ld [imm16], a
        mGameboy->writeByte(getImm16(), mRegA);
        mPC += 2;
    }
    else if (opcode == 0b11110010)
    {
        // ldh a, [c]
        mRegA = mGameboy->readByte(0xFF00 + mRegC);
    }
    else if (opcode == 0b11110000)
    {
        // ldh a, [imm8]
        mRegA = mGameboy->readByte(0xFF00 + getImm8());
        mPC += 1;
    }
    else if (opcode == 0b11111010)
    {
        // ld a, [imm16]
        mRegA = mGameboy->readByte(getImm16());
        mPC += 2;
    }
    else if (opcode == 0b11101000)
    {
        // add sp, imm8
        uint16_t orig = mSP;
        int8_t imm8 = getImm8();
        uint16_t result = mSP + imm8;
        mSP = result;
        mRegF = 0;
        if (result > 0xFF)
            mRegF |= FLAG_CARRY;
        if ((orig & (1 << 4)) != (result & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;
        mPC += 1;
    }
    else if (opcode == 0b11111000)
    {
        // ld hl, sp + imm8
        int8_t imm8 = getImm8();
        uint16_t result = mSP + imm8;
        mRegH = result >> 8;
        mRegL = result & 0xFF;
        mRegF = 0;
        if (result > 0xFF)
            mRegF |= FLAG_CARRY;
        if ((result & (1 << 4)) != (mSP & (1 << 4)))
            mRegF |= FLAG_HALFCARRY;
        mPC += 1;
    }
    else if (opcode == 0b11111001)
    {
        // ld sp, hl
        mSP = mRegH << 8 + mRegL;
    }
    else if (opcode == 0b11110011)
    {
        // di
        // TODO
    }
    else if (opcode == 0b11111011)
    {
        // ei
        // TODO
    }
    else
    {
        throw std::runtime_error("Invalid opcode");
    }
    mPC++;
}

CPU::CPUState CPU::getState() const
{
    return {mRegA, mRegF, mRegB, mRegC, mRegD, mRegE, mRegH, mRegL, mSP, mPC};
}

void CPU::setState(CPU::CPUState state)
{
    mRegA = state.regA;
    mRegF = state.regF;
    mRegB = state.regB;
    mRegC = state.regC;
    mRegD = state.regD;
    mRegE = state.regE;
    mRegH = state.regH;
    mRegL = state.regL;
    mSP = state.SP;
    mPC = state.PC;
}
