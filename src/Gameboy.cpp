#include "Gameboy.h"

#include <iostream>
#include <format>

std::string getR16Name(uint8_t bitmask)
{
    switch (bitmask)
    {
    case 0:
        return "bc";
    case 1:
        return "de";
    case 2:
        return "hl";
    case 3:
        return "sp";

    default:
        break;
    }
    return "r16?";
}

std::string getR16MemName(uint8_t bitmask)
{
    switch (bitmask)
    {
    case 0:
        return "bc";
    case 1:
        return "de";
    case 2:
        return "hl+";
    case 3:
        return "hl-";

    default:
        break;
    }
    return "r16mem?";
}

std::string getR16StkName(uint8_t bitmask)
{
    switch (bitmask)
    {
    case 0:
        return "bc";
    case 1:
        return "de";
    case 2:
        return "hl";
    case 3:
        return "af";

    default:
        break;
    }
    return "r16stk?";
}

std::string getR8Name(uint8_t bitmask)
{
    switch (bitmask)
    {
    case 0:
        return "b";
    case 1:
        return "c";
    case 2:
        return "d";
    case 3:
        return "e";
    case 4:
        return "h";
    case 5:
        return "l";
    case 6:
        return "[hl]";
    case 7:
        return "a";

    default:
        break;
    }
    return "r8?";
}

std::string getCondName(uint8_t bitmask)
{
    switch (bitmask)
    {
    case 0:
        return "nz";
    case 1:
        return "z";
    case 2:
        return "nc";
    case 3:
        return "c";

    default:
        break;
    }
    return "cond?";
}

uint16_t getImm16(const std::vector<uint8_t> &data, int i)
{
    return (data[i + 1] << 8) + data[i + 2];
}

uint8_t getImm8(const std::vector<uint8_t> &data, int i)
{
    return data[i + 1];
}

void Gameboy::dumpInstructions() const
{
    // Assumes everything is code to keep it simple
    auto data = mCart.getData();
    for (int i = 0; i < data.size(); i++)
    {
        auto opcode = data[i];
        if (opcode == 0xCB)
        {
            // Prefix
            i++;
            opcode = data[i];
            if (opcode & 0b11111000 == 0b00000000)
            {
                // rlc r8
                std::cout << std::format("rlc {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00001000)
            {
                // rrc r8
                std::cout << std::format("rrc {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00010000)
            {
                // rl r8
                std::cout << std::format("rl {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00011000)
            {
                // rr r8
                std::cout << std::format("rr {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00100000)
            {
                // sla r8
                std::cout << std::format("sla {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00101000)
            {
                // sra r8
                std::cout << std::format("sra {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00110000)
            {
                // swap r8
                std::cout << std::format("swap {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11111000 == 0b00111000)
            {
                // srl r8
                std::cout << std::format("srl {}", getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11000000 == 0b01000000)
            {
                // bit b3, r8
                std::cout << std::format("bit {}, {}", (opcode & 0b00111000) >> 3, getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11000000 == 0b10000000)
            {
                // res b3, r8
                std::cout << std::format("res {}, {}", (opcode & 0b00111000) >> 3, getR8Name(opcode & 0b00000111));
            }
            else if (opcode & 0b11000000 == 0b11000000)
            {
                // set b3, r8
                std::cout << std::format("set {}, {}", (opcode & 0b00111000) >> 3, getR8Name(opcode & 0b00000111));
            }
            else
            {
                std::cout << "INVALID OPCODE";
            }
        }
        else if (opcode == 0xD3 || opcode == 0xDB || opcode == 0xDD || opcode == 0xE3
            || opcode == 0xE4 || opcode == 0xEB || opcode == 0xEC || opcode == 0xED
            || opcode == 0xF4 || opcode == 0xFC || opcode == 0xFD)
        {
            std::cout << "INVALID OPCODE";
        }
        else if (opcode == 0)
        {
            // nop
            std::cout << "nop";
        }
        else if (opcode & 0b11001111 == 0b00000001)
        {
            // ld r16, imm16
            std::cout << std::format("ld {}, {}", getR16Name((opcode & 0b00110000) >> 4), getImm16(data, i));
            i += 2;
        }
        else if (opcode & 0b11001111 == 0b00000010)
        {
            // ld [r16mem], a
            std::cout << std::format("ld [{}], a", getR16MemName((opcode & 0b00110000) >> 4));
        }
        else if (opcode & 0b11001111 == 0b00001010)
        {
            // ld a, [r16mem]
            std::cout << std::format("ld a, [{}]", getR16MemName((opcode & 0b00110000) >> 4));
        }
        else if (opcode == 0b00001000)
        {
            // ld [imm16], sp
            std::cout << std::format("ld [{}], sp", getImm16(data, i));
            i += 2;
        }
        else if (opcode & 0b11001111 == 0b00000011)
        {
            // inc r16
            std::cout << std::format("inc {}", getR16Name((opcode & 0b00110000) >> 4));
        }
        else if (opcode & 0b11001111 == 0b00001011)
        {
            // dec r16
            std::cout << std::format("dec {}", getR16Name((opcode & 0b00110000) >> 4));
        }
        else if (opcode & 0b11001111 == 0b00001001)
        {
            // add hl, r16
            std::cout << std::format("add hl, {}", getR16Name((opcode & 0b00110000) >> 4));
        }
        else if (opcode & 0b11000111 == 0b00000100)
        {
            // inc r8
            std::cout << std::format("inc {}", getR8Name((opcode & 0b00111000) >> 3));
        }
        else if (opcode & 0b11000111 == 0b00000101)
        {
            // dec r8
            std::cout << std::format("dec {}", getR8Name((opcode & 0b00111000) >> 3));
        }
        else if (opcode & 0b11000111 == 0b00000110)
        {
            // ld r8, imm8
            std::cout << std::format("ld {}, {}", getR8Name((opcode & 0b00111000) >> 3), getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b00000111)
        {
            // rlca
            std::cout << "rlca";
        }
        else if (opcode == 0b00001111)
        {
            // rrca
            std::cout << "rrca";
        }
        else if (opcode == 0b00010111)
        {
            // rla
            std::cout << "rla";
        }
        else if (opcode == 0b00011111)
        {
            // rra
            std::cout << "rra";
        }
        else if (opcode == 0b00100111)
        {
            // daa
            std::cout << "daa";
        }
        else if (opcode == 0b00101111)
        {
            // cpl
            std::cout << "cpl";
        }
        else if (opcode == 0b00110111)
        {
            // scf
            std::cout << "scf";
        }
        else if (opcode == 0b00111111)
        {
            // ccf
            std::cout << "ccf";
        }
        else if (opcode == 0b00011000)
        {
            // jr imm8
            std::cout << std::format("jr {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode & 0b11100111 == 0b00100000)
        {
            // jr cond, imm8
            std::cout << std::format("jr {}, {}", getCondName((opcode & 0b00011000) >> 3), getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b00010000)
        {
            // stop
            std::cout << "stop";
        }
        else if (opcode == 0b01110110)
        {
            // halt (ld [hl], [hl])
            std::cout << "halt";
        }
        else if (opcode & 0b11000000 == 0b01000000)
        {
            // ld r8, r8
            std::cout << std::format("ld {}, {}", getR8Name((opcode & 0b00111000) >> 3), getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10000000)
        {
            // add a, r8
            std::cout << std::format("add a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10001000)
        {
            // adc a, r8
            std::cout << std::format("adc a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10010000)
        {
            // sub a, r8
            std::cout << std::format("sub a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10011000)
        {
            // sbc a, r8
            std::cout << std::format("sbc a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10100000)
        {
            // and a, r8
            std::cout << std::format("and a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10101000)
        {
            // xor a, r8
            std::cout << std::format("xor a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10110000)
        {
            // or a, r8
            std::cout << std::format("or a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode & 0b11111000 == 0b10111000)
        {
            // cp a, r8
            std::cout << std::format("cp a, {}", getR8Name(opcode & 0b00000111));
        }
        else if (opcode == 0b11000110)
        {
            // add a, imm8
            std::cout << std::format("add a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11001110)
        {
            // adc a, imm8
            std::cout << std::format("adc a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11010110)
        {
            // sub a, imm8
            std::cout << std::format("sub a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11011110)
        {
            // sbc a, imm8
            std::cout << std::format("sbc a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11100110)
        {
            // and a, imm8
            std::cout << std::format("and a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11101110)
        {
            // xor a, imm8
            std::cout << std::format("xor a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11110110)
        {
            // or a, imm8
            std::cout << std::format("or a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11111110)
        {
            // cp a, imm8
            std::cout << std::format("cp a, {}", getImm8(data, i));
            i += 1;
        }
        else if (opcode & 0b11100111 == 0b11000000)
        {
            // ret cond
            std::cout << std::format("ret {}", getCondName((opcode & 0b00011000) >> 3));
        }
        else if (opcode == 0b11001001)
        {
            // ret
            std::cout << "ret";
        }
        else if (opcode == 0b11011001)
        {
            // reti
            std::cout << "reti";
        }
        else if (opcode & 0b11100111 == 0b11000010)
        {
            // jp cond, imm16
            std::cout << std::format("jp {}, {}", getCondName((opcode & 0b00011000) >> 3), getImm16(data, i));
            i += 2;
        }
        else if (opcode == 0b11000011)
        {
            // jp imm16
            std::cout << std::format("jp {}", getImm16(data, i));
            i += 2;
        }
        else if (opcode == 0b11101001)
        {
            // jp hl
            std::cout << "jp hl";
        }
        else if (opcode & 0b11100111 == 0b11000100)
        {
            // call cond, imm16
            std::cout << std::format("call {}, {}", getCondName((opcode & 0b00011000) >> 3), getImm16(data, i));
            i += 2;
        }
        else if (opcode == 0b11001101)
        {
            // call imm16
            std::cout << std::format("call {}", getImm16(data, i));
            i += 2;
        }
        else if (opcode & 0b11000111 == 0b11000111)
        {
            // rst tgt3
            std::cout << std::format("rst {}", (opcode & 0b00111000) >> 3);
        }
        else if (opcode & 0b11001111 == 0b11000001)
        {
            // pop r16stk
            std::cout << std::format("pop {}", getR16StkName((opcode & 0b00110000) >> 4));
        }
        else if (opcode & 0b11001111 == 0b11000101)
        {
            // push r16stk
            std::cout << std::format("push {}", getR16StkName((opcode & 0b00110000) >> 4));
        }
        else if (opcode == 0b11100010)
        {
            // ldh [c], a
            std::cout << "ldh [c], a";
        }
        else if (opcode == 0b11100000)
        {
            // ldh [imm8], a
            std::cout << std::format("ldh [{}], a", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11101010)
        {
            // ld [imm16], a
            std::cout << std::format("ld [{}], a", getImm16(data, i));
            i += 2;
        }
        else if (opcode == 0b11110010)
        {
            // ldh a, [c]
            std::cout << "ldh a, [c]";
        }
        else if (opcode == 0b11110000)
        {
            // ldh a, [imm8]
            std::cout << std::format("ldh a, [{}]", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11111010)
        {
            // ld a, [imm16]
            std::cout << std::format("ld a, [{}]", getImm16(data, i));
            i += 2;
        }
        else if (opcode == 0b11101000)
        {
            // add sp, imm8
            std::cout << std::format("add sp, []", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11111000)
        {
            // ld hl, sp + imm8
            std::cout << std::format("ld hl, sp + []", getImm8(data, i));
            i += 1;
        }
        else if (opcode == 0b11111001)
        {
            // ld sp, hl
            std::cout << "ld sp, hl";
        }
        else if (opcode == 0b11110011)
        {
            // di
            std::cout << "di";
        }
        else if (opcode == 0b11111011)
        {
            // ei
            std::cout << "ei";
        }
        else
        {
            std::cout << "INVALID OPCODE";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}
