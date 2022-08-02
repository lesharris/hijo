#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hijo {

  class Instructions {
  public:
    enum AddressMode {
      AM_IMP,
      AM_R_D16,
      AM_R_R,
      AM_MR_R,
      AM_R,
      AM_R_D8,
      AM_R_MR,
      AM_R_HLI,
      AM_R_HLD,
      AM_HLI_R,
      AM_HLD_R,
      AM_R_A8,
      AM_A8_R,
      AM_HL_SPR,
      AM_D16,
      AM_D8,
      AM_D16_R,
      AM_MR_D8,
      AM_MR,
      AM_A16_R,
      AM_R_A16
    };

    enum RegisterType {
      RT_NONE,
      RT_A,
      RT_F,
      RT_B,
      RT_C,
      RT_D,
      RT_E,
      RT_H,
      RT_L,
      RT_AF,
      RT_BC,
      RT_DE,
      RT_HL,
      RT_SP,
      RT_PC
    };

    enum InstructionType {
      IN_NONE,
      IN_NOP,
      IN_LD,
      IN_INC,
      IN_DEC,
      IN_RLCA,
      IN_ADD,
      IN_RRCA,
      IN_STOP,
      IN_RLA,
      IN_JR,
      IN_RRA,
      IN_DAA,
      IN_CPL,
      IN_SCF,
      IN_CCF,
      IN_HALT,
      IN_ADC,
      IN_SUB,
      IN_SBC,
      IN_AND,
      IN_XOR,
      IN_OR,
      IN_CP,
      IN_POP,
      IN_JP,
      IN_PUSH,
      IN_RET,
      IN_CB,
      IN_CALL,
      IN_RETI,
      IN_LDH,
      IN_JPHL,
      IN_DI,
      IN_EI,
      IN_RST,
      IN_ERR,
      //CB instructions...
      IN_RLC,
      IN_RRC,
      IN_RL,
      IN_RR,
      IN_SLA,
      IN_SRA,
      IN_SWAP,
      IN_SRL,
      IN_BIT,
      IN_RES,
      IN_SET
    };

    enum ConditionType {
      CT_NONE, CT_NZ, CT_Z, CT_NC, CT_C
    };

    struct Instruction {
      Instruction(InstructionType type) : type(type) {}

      Instruction(InstructionType type, AddressMode mode) : type(type), mode(mode) {}

      Instruction(InstructionType type, AddressMode mode, RegisterType r1type)
          : type(type), mode(mode), reg1(r1type) {}

      Instruction(InstructionType type, AddressMode mode, RegisterType r1type, RegisterType r2type)
          : type(type), mode(mode), reg1(r1type), reg2(r2type) {}

      Instruction(InstructionType type, AddressMode mode, RegisterType r1type, RegisterType r2type, ConditionType cond)
          : type(type), mode(mode), reg1(r1type), reg2(r2type), cond(cond) {}

      Instruction(InstructionType type, AddressMode mode, RegisterType r1type, RegisterType r2type, ConditionType cond,
                  uint8_t param)
          : type(type), mode(mode), reg1(r1type), reg2(r2type), cond(cond), param(param) {}

      InstructionType type;
      AddressMode mode;
      RegisterType reg1;
      RegisterType reg2;
      ConditionType cond;
      uint8_t param;
    };

    Instruction &InstructionByOpcode(uint8_t opcode);

    const std::string &InstructionName(const Instruction &t);

    std::string InstructionToString(const Instruction &t);

  private:
    std::vector<Instructions::Instruction> m_Instructions;
    std::vector<std::string> m_InstructionNames;
    std::vector<std::string> m_RegisterNames;
  };

} // hijo
