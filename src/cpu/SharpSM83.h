#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>

#include "common/common.h"
#include "Instructions.h"

#include "Stack.h"

namespace hijo {

  class SharpSM83 {
  public:
    struct Registers {
      uint8_t a;
      uint8_t f;
      uint8_t b;
      uint8_t c;
      uint8_t d;
      uint8_t e;
      uint8_t h;
      uint8_t l;
      uint16_t pc;
      uint16_t sp;
    };

  public:
    SharpSM83();

    bool Step();

    void Reset();

    uint16_t Register(const Instructions::RegisterType &t);

    void Register(const Instructions::RegisterType &t, uint16_t value);

    uint8_t IERegister() {
      return ie_register;
    }

    void IERegister(uint8_t data) {
      ie_register = data;
    }

    uint8_t IntFlags() {
      return int_flags;
    }

    void IntFlags(uint8_t value) {
      int_flags = value;
    }

    uint8_t Register8(const Instructions::RegisterType &t);

    void Register8(const Instructions::RegisterType &t, uint8_t value);

  private:
    friend class Gameboy;

    friend class UI;

    friend class Interrupts;

    friend class DMA;

    friend class Timer;

    friend class Stack;

  private:
    using InstructionProc = void (SharpSM83::*)();

  private:
    void FetchData();

    void FetchInstruction();

    void Execute();

    void SetFlags(int8_t z, int8_t n, int8_t h, int8_t c);

    Instructions::RegisterType DecodeRegister(uint8_t reg);

    bool Is16Bit(const Instructions::RegisterType &t);

    bool CheckCondition();

    void GotoAddress(uint16_t addr, bool pushPC);

    InstructionProc GetProcessor(Instructions::InstructionType type);

    uint16_t reverse(uint16_t n);

  private:
    void ProcNone();

    void ProcNOP();

    void ProcCB();

    void ProcRLCA();

    void ProcRRCA();

    void ProcRLA();

    void ProcSTOP();

    void ProcDAA();

    void ProcCPL();

    void ProcSCF();

    void ProcCCF();

    void ProcHALT();

    void ProcRRA();

    void ProcAND();

    void ProcXOR();

    void ProcOR();

    void ProcCP();

    void ProcDI();

    void ProcEI();

    void ProcLD();

    void ProcLDH();

    void ProcJP();

    void ProcJR();

    void ProcCALL();

    void ProcRST();

    void ProcRET();

    void ProcRETI();

    void ProcPOP();

    void ProcPUSH();

    void ProcINC();

    void ProcDEC();

    void ProcSUB();

    void ProcSBC();

    void ProcADC();

    void ProcADD();

#define CPU_FLAG_Z BIT(regs.f, 7)
#define CPU_FLAG_N BIT(regs.f, 6)
#define CPU_FLAG_H BIT(regs.f, 5)
#define CPU_FLAG_C BIT(regs.f, 4)

  private:
    Registers regs;
    Instructions instrs;

    uint16_t fetched_data;
    uint16_t mem_dest;
    bool dest_is_mem;
    uint8_t cur_opcode;
    Instructions::Instruction *cur_inst;

    bool halted;
    bool stepping;

    bool int_master_enabled;
    bool enabling_ime;
    uint8_t ie_register;
    uint8_t int_flags;

    std::vector<Instructions::RegisterType> rt_lookup{
        Instructions::RegisterType::RT_B,
        Instructions::RegisterType::RT_C,
        Instructions::RegisterType::RT_D,
        Instructions::RegisterType::RT_E,
        Instructions::RegisterType::RT_H,
        Instructions::RegisterType::RT_L,
        Instructions::RegisterType::RT_HL,
        Instructions::RegisterType::RT_A
    };

    std::unordered_map<Instructions::InstructionType, InstructionProc> m_Processors{
        {Instructions::InstructionType::IN_NONE, &SharpSM83::ProcNone},
        {Instructions::InstructionType::IN_NOP,  &SharpSM83::ProcNOP},
        {Instructions::InstructionType::IN_LD,   &SharpSM83::ProcLD},
        {Instructions::InstructionType::IN_LDH,  &SharpSM83::ProcLDH},
        {Instructions::InstructionType::IN_JP,   &SharpSM83::ProcJP},
        {Instructions::InstructionType::IN_DI,   &SharpSM83::ProcDI},
        {Instructions::InstructionType::IN_POP,  &SharpSM83::ProcPOP},
        {Instructions::InstructionType::IN_PUSH, &SharpSM83::ProcPUSH},
        {Instructions::InstructionType::IN_JR,   &SharpSM83::ProcJR},
        {Instructions::InstructionType::IN_CALL, &SharpSM83::ProcCALL},
        {Instructions::InstructionType::IN_RET,  &SharpSM83::ProcRET},
        {Instructions::InstructionType::IN_RST,  &SharpSM83::ProcRST},
        {Instructions::InstructionType::IN_DEC,  &SharpSM83::ProcDEC},
        {Instructions::InstructionType::IN_INC,  &SharpSM83::ProcINC},
        {Instructions::InstructionType::IN_ADD,  &SharpSM83::ProcADD},
        {Instructions::InstructionType::IN_ADC,  &SharpSM83::ProcADC},
        {Instructions::InstructionType::IN_SUB,  &SharpSM83::ProcSUB},
        {Instructions::InstructionType::IN_SBC,  &SharpSM83::ProcSBC},
        {Instructions::InstructionType::IN_AND,  &SharpSM83::ProcAND},
        {Instructions::InstructionType::IN_XOR,  &SharpSM83::ProcXOR},
        {Instructions::InstructionType::IN_OR,   &SharpSM83::ProcOR},
        {Instructions::InstructionType::IN_CP,   &SharpSM83::ProcCP},
        {Instructions::InstructionType::IN_CB,   &SharpSM83::ProcCB},
        {Instructions::InstructionType::IN_RRCA, &SharpSM83::ProcRRCA},
        {Instructions::InstructionType::IN_RLCA, &SharpSM83::ProcRLCA},
        {Instructions::InstructionType::IN_RRA,  &SharpSM83::ProcRRA},
        {Instructions::InstructionType::IN_RLA,  &SharpSM83::ProcRLA},
        {Instructions::InstructionType::IN_STOP, &SharpSM83::ProcSTOP},
        {Instructions::InstructionType::IN_HALT, &SharpSM83::ProcHALT},
        {Instructions::InstructionType::IN_DAA,  &SharpSM83::ProcDAA},
        {Instructions::InstructionType::IN_CPL,  &SharpSM83::ProcCPL},
        {Instructions::InstructionType::IN_SCF,  &SharpSM83::ProcSCF},
        {Instructions::InstructionType::IN_CCF,  &SharpSM83::ProcCCF},
        {Instructions::InstructionType::IN_EI,   &SharpSM83::ProcEI},
        {Instructions::InstructionType::IN_RETI, &SharpSM83::ProcRETI}
    };
  };

} // hijo
