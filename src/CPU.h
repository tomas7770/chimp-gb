#pragma once

#include <cstdint>
#include <memory>

class Gameboy;

class CPU
{
public:
    CPU(Gameboy *gameboy, bool debug) : mGameboy(gameboy), mDebugPrint(debug) {}

    void doMCycle();

    enum InterruptSource
    {
        VBlank,
        LCD,
        Timer,
        Serial,
        Joypad
    };
    void requestInterrupt(InterruptSource source);
    void startDMATransfer(uint8_t value);
    void startGeneralDMA(int hdmaLength);
    void startHBlankDMA(int hdmaLength);
    uint8_t getDMALengthLeft();

    void loadBootRom();
    void simulateBootRom();

    // Interrupts (these registers are accessible through memory map)
    // Interrupt enable
    uint8_t IE = 0;
    // Interrupt flag
    uint8_t IF = 0;

    uint16_t hdmaSrc;
    uint16_t hdmaDest = 0x8000;

private:
    uint8_t readByteAtPC() const;

    uint8_t readR8(uint8_t bitmask) const;
    void writeR8(uint8_t bitmask, uint8_t value);
    uint8_t readByteAtHL() const;
    void writeByteAtHL(uint8_t value);
    uint16_t readR16(uint8_t bitmask) const;
    void writeR16(uint8_t bitmask, uint16_t value);
    uint16_t readR16Mem(uint8_t bitmask);
    bool checkCond(uint8_t bitmask) const;
    uint8_t readR16StkLow(uint8_t bitmask) const;
    uint8_t readR16StkHigh(uint8_t bitmask) const;
    void writeR16StkLow(uint8_t bitmask, uint8_t value);
    void writeR16StkHigh(uint8_t bitmask, uint8_t value);

    void opcode_nop_M1();
    void opcode_ldr8r8_M1();
    void opcode_ldr8imm8_M1();
    void opcode_ldr8imm8_M2();
    void opcode_ldr8ihl_M1();
    void opcode_ldr8ihl_M2();
    void opcode_ldihlr8_M1();
    void opcode_ldihlr8_M2();
    void opcode_ldihlimm8_M1();
    void opcode_ldihlimm8_M2();
    void opcode_ldihlimm8_M3();
    void opcode_ldair16mem_M1();
    void opcode_ldair16mem_M2();
    void opcode_ldir16mema_M1();
    void opcode_ldir16mema_M2();
    void opcode_ldaiimm16_M1();
    void opcode_ldaiimm16_M2();
    void opcode_ldaiimm16_M3();
    void opcode_ldaiimm16_M4();
    void opcode_ldiimm16a_M1();
    void opcode_ldiimm16a_M2();
    void opcode_ldiimm16a_M3();
    void opcode_ldiimm16a_M4();
    void opcode_ldhaic_M1();
    void opcode_ldhaic_M2();
    void opcode_ldhica_M1();
    void opcode_ldhica_M2();
    void opcode_ldhaiimm8_M1();
    void opcode_ldhaiimm8_M2();
    void opcode_ldhaiimm8_M3();
    void opcode_ldhiimm8a_M1();
    void opcode_ldhiimm8a_M2();
    void opcode_ldhiimm8a_M3();
    void opcode_ldr16imm16_M1();
    void opcode_ldr16imm16_M2();
    void opcode_ldr16imm16_M3();
    void opcode_ldiimm16sp_M1();
    void opcode_ldiimm16sp_M2();
    void opcode_ldiimm16sp_M3();
    void opcode_ldiimm16sp_M4();
    void opcode_ldiimm16sp_M5();
    void opcode_ldsphl_M1();
    void opcode_ldsphl_M2();
    void opcode_pushr16stk_M1();
    void opcode_pushr16stk_M2();
    void opcode_pushr16stk_M3();
    void opcode_pushr16stk_M4();
    void opcode_popr16stk_M1();
    void opcode_popr16stk_M2();
    void opcode_popr16stk_M3();
    void opcode_ldhlspimm8_M1();
    void opcode_ldhlspimm8_M2();
    void opcode_ldhlspimm8_M3();
    void opcode_addar8_M1();
    void opcode_addaihl_M1();
    void opcode_addaihl_M2();
    void opcode_addaimm8_M1();
    void opcode_addaimm8_M2();
    void opcode_adcar8_M1();
    void opcode_adcaihl_M1();
    void opcode_adcaihl_M2();
    void opcode_adcaimm8_M1();
    void opcode_adcaimm8_M2();
    void opcode_subar8_M1();
    void opcode_subaihl_M1();
    void opcode_subaihl_M2();
    void opcode_subaimm8_M1();
    void opcode_subaimm8_M2();
    void opcode_sbcar8_M1();
    void opcode_sbcaihl_M1();
    void opcode_sbcaihl_M2();
    void opcode_sbcaimm8_M1();
    void opcode_sbcaimm8_M2();
    void opcode_cpar8_M1();
    void opcode_cpaihl_M1();
    void opcode_cpaihl_M2();
    void opcode_cpaimm8_M1();
    void opcode_cpaimm8_M2();
    void opcode_incr8_M1();
    void opcode_incihl_M1();
    void opcode_incihl_M2();
    void opcode_incihl_M3();
    void opcode_decr8_M1();
    void opcode_decihl_M1();
    void opcode_decihl_M2();
    void opcode_decihl_M3();
    void opcode_andar8_M1();
    void opcode_andaihl_M1();
    void opcode_andaihl_M2();
    void opcode_andaimm8_M1();
    void opcode_andaimm8_M2();
    void opcode_orar8_M1();
    void opcode_oraihl_M1();
    void opcode_oraihl_M2();
    void opcode_oraimm8_M1();
    void opcode_oraimm8_M2();
    void opcode_xorar8_M1();
    void opcode_xoraihl_M1();
    void opcode_xoraihl_M2();
    void opcode_xoraimm8_M1();
    void opcode_xoraimm8_M2();
    void opcode_ccf_M1();
    void opcode_scf_M1();
    void opcode_daa_M1();
    void opcode_cpl_M1();
    void opcode_incr16_M1();
    void opcode_incr16_M2();
    void opcode_decr16_M1();
    void opcode_decr16_M2();
    void opcode_addhlr16_M1();
    void opcode_addhlr16_M2();
    void opcode_addspimm8_M1();
    void opcode_addspimm8_M2();
    void opcode_addspimm8_M3();
    void opcode_addspimm8_M4();
    void opcode_rlca_M1();
    void opcode_rrca_M1();
    void opcode_rla_M1();
    void opcode_rra_M1();
    void opcode_rlcr8_M2();
    void opcode_rlcihl_M2();
    void opcode_rlcihl_M3();
    void opcode_rlcihl_M4();
    void opcode_rrcr8_M2();
    void opcode_rrcihl_M2();
    void opcode_rrcihl_M3();
    void opcode_rrcihl_M4();
    void opcode_rlr8_M2();
    void opcode_rlihl_M2();
    void opcode_rlihl_M3();
    void opcode_rlihl_M4();
    void opcode_rrr8_M2();
    void opcode_rrihl_M2();
    void opcode_rrihl_M3();
    void opcode_rrihl_M4();
    void opcode_slar8_M2();
    void opcode_slaihl_M2();
    void opcode_slaihl_M3();
    void opcode_slaihl_M4();
    void opcode_srar8_M2();
    void opcode_sraihl_M2();
    void opcode_sraihl_M3();
    void opcode_sraihl_M4();
    void opcode_swapr8_M2();
    void opcode_swapihl_M2();
    void opcode_swapihl_M3();
    void opcode_swapihl_M4();
    void opcode_srlr8_M2();
    void opcode_srlihl_M2();
    void opcode_srlihl_M3();
    void opcode_srlihl_M4();
    void opcode_bitb3r8_M2();
    void opcode_bitb3ihl_M2();
    void opcode_bitb3ihl_M3();
    void opcode_resb3r8_M2();
    void opcode_resb3ihl_M2();
    void opcode_resb3ihl_M3();
    void opcode_resb3ihl_M4();
    void opcode_setb3r8_M2();
    void opcode_setb3ihl_M2();
    void opcode_setb3ihl_M3();
    void opcode_setb3ihl_M4();
    void opcode_jpimm16_M1();
    void opcode_jpimm16_M2();
    void opcode_jpimm16_M3();
    void opcode_jpimm16_M4();
    void opcode_jphl_M1();
    void opcode_jrimm8_M1();
    void opcode_jrimm8_M2();
    void opcode_jrimm8_M3();
    void opcode_callimm16_M1();
    void opcode_callimm16_M2();
    void opcode_callimm16_M3();
    void opcode_callimm16_M4();
    void opcode_callimm16_M5();
    void opcode_callimm16_M6();
    void opcode_ret_M1();
    void opcode_ret_M2();
    void opcode_ret_M3();
    void opcode_ret_M4();
    void opcode_reti_M1();
    void opcode_reti_M2();
    void opcode_reti_M3();
    void opcode_reti_M4();
    void opcode_rsttgt3_M1();
    void opcode_rsttgt3_M2();
    void opcode_rsttgt3_M3();
    void opcode_rsttgt3_M4();
    void opcode_di_M1();
    void opcode_ei_M1();
    void opcode_jpcondimm16_M1();
    void opcode_jpcondimm16_M2();
    void opcode_jpcondimm16_M3();
    void opcode_jpcondimm16_M4();
    void opcode_jrcondimm8_M1();
    void opcode_jrcondimm8_M2();
    void opcode_jrcondimm8_M3();
    void opcode_callcondimm16_M1();
    void opcode_callcondimm16_M2();
    void opcode_callcondimm16_M3();
    void opcode_callcondimm16_M4();
    void opcode_callcondimm16_M5();
    void opcode_callcondimm16_M6();
    void opcode_retcond_M1();
    void opcode_retcond_M2();
    void opcode_retcond_M3();
    void opcode_retcond_M4();
    void opcode_retcond_M5();
    void opcode_halt_M1();
    void opcode_prefix_cb_M1();

    void interrupt_M1();
    void interrupt_M2();
    void interrupt_M3();
    void interrupt_M4();
    void interrupt_M5();

    void decodeExecuteOpcodeCB();
    void decodeExecuteOpcode();
    void prefetchOpcode();

    void copyDMABytes(int count);

    Gameboy *mGameboy;

    // AF
    uint8_t mRegA = 0x01;
    uint8_t mRegF;
    // BC
    uint8_t mRegB = 0x00;
    uint8_t mRegC = 0x13;
    // DE
    uint8_t mRegD = 0x00;
    uint8_t mRegE = 0xD8;
    // HL
    uint8_t mRegH = 0x01;
    uint8_t mRegL = 0x4D;
    // SP and PC
    uint16_t mSP = 0xFFFE;
    uint16_t mPC = 0x0100;
    // Instruction register
    // Instruction loop is decode-execute-prefetch. PC is ahead of current instruction by 1.
    // Initially loaded instruction is a NOP.
    uint8_t mOpcode = 0;
    // Data bus
    uint8_t mDataZ;
    uint8_t mDataW;
    // Function to call on next M-cycle
    void (CPU::*mMCycleFunc)() = &CPU::decodeExecuteOpcode;

    bool mDebugPrint;

    // Interrupts
    // Interrupt master enable (disabled by default)
    bool mIME = false;
    // The effect of ei is delayed by one M-cycle. This variable counts how many M-cycles are left to enable mIME.
    int mRequestIME = 0;
    // Store handler of interrupt that was triggered
    uint16_t mIRQ;

    // Halt
    bool mHalted = false;

    // OAM DMA
    uint16_t mDMASourceStart;
    int mDMACycles;
    bool mDMACopying = false;

    // CGB DMA
    int mHDMALength;
    bool mGeneralDMACopying = false;
    bool mHBlankDMACopying = false;
    int mDMABytesCopied;
    int mHBlankBytesCopied;

    static constexpr uint8_t FLAG_ZERO = (1 << 7);
    static constexpr uint8_t FLAG_SUB = (1 << 6);
    static constexpr uint8_t FLAG_HALFCARRY = (1 << 5);
    static constexpr uint8_t FLAG_CARRY = (1 << 4);

    static constexpr int HL_BITMASK = 6;

    static constexpr uint16_t INTERRUPT_HANDLERS[] = {0x40, 0x48, 0x50, 0x58, 0x60};

    static constexpr int DMA_M_CYCLES = 160;
};
