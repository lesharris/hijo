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

    struct DisassemblyLine {
      uint16_t addr;
      uint16_t index;
      std::string text;
      std::string bytes;
      std::string mode;
    };

  public:
    SharpSM83();

    bool Step();

    void Reset();

    uint16_t Reg(const Register &t);

    void Reg(const Register &t, uint16_t value);

    uint8_t Reg8(const Register &t);

    void Reg8(const Register &t, uint8_t value);

    uint8_t IERegister() {
      return m_IE;
    }

    void IERegister(uint8_t data) {
      m_IE = data;
    }

    uint8_t IntFlags() {
      return m_IF;
    }

    void IntFlags(uint8_t value) {
      m_IF = value;
    }

    void Disassemble(uint16_t start_addr, uint16_t end_addr);

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

    Register DecodeRegister(uint8_t reg);

    bool Is16Bit(const Register &t);

    bool CheckCondition();

    void GotoAddress(uint16_t addr, bool pushPC);

    InstructionProc GetProcessor(Instruction type);

    uint16_t reverse(uint16_t n);

    void Cycle(uint8_t cycles);

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

    uint8_t CPU_FLAG_Z() {
      return BIT(regs.f, 7);
    }

    uint8_t CPU_FLAG_N() {
      return BIT(regs.f, 6);
    }

    uint8_t CPU_FLAG_H() {
      return BIT(regs.f, 5);
    }

    uint8_t CPU_FLAG_C() {
      return BIT(regs.f, 4);
    }

  private:
    Registers regs;
    Instructions instrs;

    uint16_t m_FetchedData;
    uint16_t m_MemoryDestination;
    bool DestinationIsMemory;
    uint8_t m_CurrentOpcode;
    uint8_t m_CurrentCycles;
    Instructions::Opcode *m_CurrentInstruction;

    std::vector<DisassemblyLine> m_Disassembly;

    bool m_Halted;
    bool m_Stepping;

    bool m_InterruptMasterEnabled;
    bool m_EnablingIME;
    uint8_t m_IE;
    uint8_t m_IF;

    std::vector<Register> m_RegisterTypes{
        Register::B,
        Register::C,
        Register::D,
        Register::E,
        Register::H,
        Register::L,
        Register::HL,
        Register::A
    };

    std::unordered_map<Instruction, InstructionProc> m_Processors{
        {Instruction::NONE, &SharpSM83::ProcNone},
        {Instruction::NOP,  &SharpSM83::ProcNOP},
        {Instruction::LD,   &SharpSM83::ProcLD},
        {Instruction::LDH,  &SharpSM83::ProcLDH},
        {Instruction::JP,   &SharpSM83::ProcJP},
        {Instruction::DI,   &SharpSM83::ProcDI},
        {Instruction::POP,  &SharpSM83::ProcPOP},
        {Instruction::PUSH, &SharpSM83::ProcPUSH},
        {Instruction::JR,   &SharpSM83::ProcJR},
        {Instruction::CALL, &SharpSM83::ProcCALL},
        {Instruction::RET,  &SharpSM83::ProcRET},
        {Instruction::RST,  &SharpSM83::ProcRST},
        {Instruction::DEC,  &SharpSM83::ProcDEC},
        {Instruction::INC,  &SharpSM83::ProcINC},
        {Instruction::ADD,  &SharpSM83::ProcADD},
        {Instruction::ADC,  &SharpSM83::ProcADC},
        {Instruction::SUB,  &SharpSM83::ProcSUB},
        {Instruction::SBC,  &SharpSM83::ProcSBC},
        {Instruction::AND,  &SharpSM83::ProcAND},
        {Instruction::XOR,  &SharpSM83::ProcXOR},
        {Instruction::OR,   &SharpSM83::ProcOR},
        {Instruction::CP,   &SharpSM83::ProcCP},
        {Instruction::CB,   &SharpSM83::ProcCB},
        {Instruction::RRCA, &SharpSM83::ProcRRCA},
        {Instruction::RLCA, &SharpSM83::ProcRLCA},
        {Instruction::RRA,  &SharpSM83::ProcRRA},
        {Instruction::RLA,  &SharpSM83::ProcRLA},
        {Instruction::STOP, &SharpSM83::ProcSTOP},
        {Instruction::HALT, &SharpSM83::ProcHALT},
        {Instruction::DAA,  &SharpSM83::ProcDAA},
        {Instruction::CPL,  &SharpSM83::ProcCPL},
        {Instruction::SCF,  &SharpSM83::ProcSCF},
        {Instruction::CCF,  &SharpSM83::ProcCCF},
        {Instruction::EI,   &SharpSM83::ProcEI},
        {Instruction::RETI, &SharpSM83::ProcRETI}
    };
  };

} // hijo
