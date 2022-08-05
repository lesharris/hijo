#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace hijo {
  enum class AddressMode {
    IMP,
    R_D16,
    R_R,
    MR_R,
    R,
    R_D8,
    R_MR,
    R_HLI,
    R_HLD,
    HLI_R,
    HLD_R,
    R_A8,
    A8_R,
    HL_SPR,
    D16,
    D8,
    D16_R,
    MR_D8,
    MR,
    A16_R,
    R_A16
  };

  enum class Register {
    NONE,
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
    SP,
    PC
  };

  enum class Instruction {
    NONE,
    NOP,
    LD,
    INC,
    DEC,
    RLCA,
    ADD,
    RRCA,
    STOP,
    RLA,
    JR,
    RRA,
    DAA,
    CPL,
    SCF,
    CCF,
    HALT,
    ADC,
    SUB,
    SBC,
    AND,
    XOR,
    OR,
    CP,
    POP,
    JP,
    PUSH,
    RET,
    CB,
    CALL,
    RETI,
    LDH,
    JPHL,
    DI,
    EI,
    RST,
    ERR,
    //CB instructions...
    RLC,
    RRC,
    RL,
    RR,
    SLA,
    SRA,
    SWAP,
    SRL,
    BIT,
    RES,
    SET
  };

  enum class Condition {
    NONE,
    NZ,
    Z,
    NC,
    C
  };

  class Instructions {
  public:
    struct Opcode {
      uint8_t code = 0;
      uint8_t length = 0;
      std::string name = "NOP";
      Instruction type = Instruction::NOP;
      AddressMode mode = AddressMode::IMP;
      Register reg1 = Register::NONE;
      Register reg2 = Register::NONE;
      Condition cond = Condition::NONE;
      uint8_t param = 0;
    };

    Opcode &OpcodeByByte(uint8_t opcode) {
      return m_Instructions[opcode];
    }

    const std::string &AddressModeLabel(const AddressMode &am) {
      return m_AddressModeNames[am];
    }

    const std::string &RegisterLabel(const Register &r) {
      return m_RegisterNames[r];
    }

  private:
    std::vector<Instructions::Opcode> m_Instructions = {
        {0x0,  1, "NOP",               Instruction::NOP,  AddressMode::IMP},
        {0x1,  3, "LD BC, u16",        Instruction::LD,   AddressMode::R_D16,  Register::BC},
        {0x2,  1, "LD (BC), A",        Instruction::LD,   AddressMode::MR_R,   Register::BC,   Register::A},
        {0x3,  1, "INC BC",            Instruction::INC,  AddressMode::R,      Register::BC},
        {0x4,  1, "INC B",             Instruction::INC,  AddressMode::R,      Register::B},
        {0x5,  1, "DEC B",             Instruction::DEC,  AddressMode::R,      Register::B},
        {0x6,  2, "LD B, u8",          Instruction::LD,   AddressMode::R_D8,   Register::B},
        {0x7,  1, "RLCA",              Instruction::RLCA},
        {0x8,  3, "LD (u16), SP",      Instruction::LD,   AddressMode::A16_R,  Register::NONE, Register::SP},
        {0x9,  1, "ADD HL, BC",        Instruction::ADD,  AddressMode::R_R,    Register::HL,   Register::BC},
        {0xA,  1, "LD A, (BC)",        Instruction::LD,   AddressMode::R_MR,   Register::A,    Register::BC},
        {0xB,  1, "DEC BC",            Instruction::DEC,  AddressMode::R,      Register::BC},
        {0xC,  1, "INC C",             Instruction::INC,  AddressMode::R,      Register::C},
        {0xD,  1, "DEC C",             Instruction::DEC,  AddressMode::R,      Register::C},
        {0xE,  2, "LD C, u8",          Instruction::LD,   AddressMode::R_D8,   Register::C},
        {0xF,  1, "RRCA",              Instruction::RRCA},

        //0x1X
        {0x10, 1, "STOP",              Instruction::STOP},
        {0x11, 3, "LD DE, u16",        Instruction::LD,   AddressMode::R_D16,  Register::DE},
        {0x12, 1, "LD (DE), A",        Instruction::LD,   AddressMode::MR_R,   Register::DE,   Register::A},
        {0x13, 1, "INC DE",            Instruction::INC,  AddressMode::R,      Register::DE},
        {0x14, 1, "INC D",             Instruction::INC,  AddressMode::R,      Register::D},
        {0x15, 1, "DEC D",             Instruction::DEC,  AddressMode::R,      Register::D},
        {0x16, 2, "LD D, u8",          Instruction::LD,   AddressMode::R_D8,   Register::D},
        {0x17, 1, "RLA",               Instruction::RLA},
        {0x18, 2, "JR i8",             Instruction::JR,   AddressMode::D8},
        {0x19, 1, "ADD HL, DE",        Instruction::ADD,  AddressMode::R_R,    Register::HL,   Register::DE},
        {0x1A, 1, "LD A, (DE)",        Instruction::LD,   AddressMode::R_MR,   Register::A,    Register::DE},
        {0x1B, 1, "DEC DE",            Instruction::DEC,  AddressMode::R,      Register::DE},
        {0x1C, 1, "INC E",             Instruction::INC,  AddressMode::R,      Register::E},
        {0x1D, 1, "DEC E",             Instruction::DEC,  AddressMode::R,      Register::E},
        {0x1E, 2, "LD E, u8",          Instruction::LD,   AddressMode::R_D8,   Register::E},
        {0x1F, 1, "RRA",               Instruction::RRA},

        //0x2X
        {0x20, 2, "JR NZ, i8",         Instruction::JR,   AddressMode::D8,     Register::NONE, Register::NONE, Condition::NZ},
        {0x21, 3, "LD HL, u16",        Instruction::LD,   AddressMode::R_D16,  Register::HL},
        {0x22, 1, "LD (HL+), A",       Instruction::LD,   AddressMode::HLI_R,  Register::HL,   Register::A},
        {0x23, 1, "INC HL",            Instruction::INC,  AddressMode::R,      Register::HL},
        {0x24, 1, "INC H",             Instruction::INC,  AddressMode::R,      Register::H},
        {0x25, 1, "DEC H",             Instruction::DEC,  AddressMode::R,      Register::H},
        {0x26, 2, "LD H, u8",          Instruction::LD,   AddressMode::R_D8,   Register::H},
        {0x27, 1, "DAA",               Instruction::DAA},
        {0x28, 2, "JR Z, i8",          Instruction::JR,   AddressMode::D8,     Register::NONE, Register::NONE, Condition::Z},
        {0x29, 1, "ADD HL, HL",        Instruction::ADD,  AddressMode::R_R,    Register::HL,   Register::HL},
        {0x2A, 1, "LD A, (HL+)",       Instruction::LD,   AddressMode::R_HLI,  Register::A,    Register::HL},
        {0x2B, 1, "DEC HL",            Instruction::DEC,  AddressMode::R,      Register::HL},
        {0x2C, 1, "INC L",             Instruction::INC,  AddressMode::R,      Register::L},
        {0x2D, 1, "DEC L",             Instruction::DEC,  AddressMode::R,      Register::L},
        {0x2E, 2, "LD L, u8",          Instruction::LD,   AddressMode::R_D8,   Register::L},
        {0x2F, 1, "CPL",               Instruction::CPL},

        //0x3X
        {0x30, 2, "JR NC, i8",         Instruction::JR,   AddressMode::D8,     Register::NONE, Register::NONE, Condition::NC},
        {0x31, 3, "LD SP, u16",        Instruction::LD,   AddressMode::R_D16,  Register::SP},
        {0x32, 1, "LD (HL-), A",       Instruction::LD,   AddressMode::HLD_R,  Register::HL,   Register::A},
        {0x33, 1, "INC SP",            Instruction::INC,  AddressMode::R,      Register::SP},
        {0x34, 1, "INC (HL)",          Instruction::INC,  AddressMode::MR,     Register::HL},
        {0x35, 1, "DEC (HL)",          Instruction::DEC,  AddressMode::MR,     Register::HL},
        {0x36, 2, "LD (HL), u8",       Instruction::LD,   AddressMode::MR_D8,  Register::HL},
        {0x37, 1, "SCF",               Instruction::SCF},
        {0x38, 2, "JR C, i8",          Instruction::JR,   AddressMode::D8,     Register::NONE, Register::NONE, Condition::C},
        {0x39, 1, "ADD HL, SP",        Instruction::ADD,  AddressMode::R_R,    Register::HL,   Register::SP},
        {0x3A, 1, "LD A, (HL-)",       Instruction::LD,   AddressMode::R_HLD,  Register::A,    Register::HL},
        {0x3B, 1, "DEC SP",            Instruction::DEC,  AddressMode::R,      Register::SP},
        {0x3C, 1, "INC A",             Instruction::INC,  AddressMode::R,      Register::A},
        {0x3D, 1, "DEC A",             Instruction::DEC,  AddressMode::R,      Register::A},
        {0x3E, 2, "LD A, u8",          Instruction::LD,   AddressMode::R_D8,   Register::A},
        {0x3F, 1, "CCF",               Instruction::CCF},

        //0x4X
        {0x40, 1, "LD B, B",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::B},
        {0x41, 1, "LD B, C",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::C},
        {0x42, 1, "LD B, D",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::D},
        {0x43, 1, "LD B, E",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::E},
        {0x44, 1, "LD B, H",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::H},
        {0x45, 1, "LD B, L",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::L},
        {0x46, 1, "LD B, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::B,    Register::HL},
        {0x47, 1, "LD B, A",           Instruction::LD,   AddressMode::R_R,    Register::B,    Register::A},
        {0x48, 1, "LD C, B",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::B},
        {0x49, 1, "LD C, C",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::C},
        {0x4A, 1, "LD C, D",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::D},
        {0x4B, 1, "LD C, E",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::E},
        {0x4C, 1, "LD C, H",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::H},
        {0x4D, 1, "LD C, L",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::L},
        {0x4E, 1, "LD C, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::C,    Register::HL},
        {0x4F, 1, "LD C, A",           Instruction::LD,   AddressMode::R_R,    Register::C,    Register::A},

        //0x5X
        {0x50, 1, "LD D, B",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::B},
        {0x51, 1, "LD D, C",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::C},
        {0x52, 1, "LD D, D",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::D},
        {0x53, 1, "LD D, E",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::E},
        {0x54, 1, "LD D, H",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::H},
        {0x55, 1, "LD D, L",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::L},
        {0x56, 1, "LD D, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::D,    Register::HL},
        {0x57, 1, "LD D, A",           Instruction::LD,   AddressMode::R_R,    Register::D,    Register::A},
        {0x58, 1, "LD E, B",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::B},
        {0x59, 1, "LD E, C",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::C},
        {0x5A, 1, "LD E, D",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::D},
        {0x5B, 1, "LD E, E",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::E},
        {0x5C, 1, "LD E, H",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::H},
        {0x5D, 1, "LD E, L",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::L},
        {0x5E, 1, "LD E, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::E,    Register::HL},
        {0x5F, 1, "LD E, A",           Instruction::LD,   AddressMode::R_R,    Register::E,    Register::A},

        //0x6X
        {0x60, 1, "LD H, B",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::B},
        {0x61, 1, "LD H, C",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::C},
        {0x62, 1, "LD H, D",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::D},
        {0x63, 1, "LD H, E",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::E},
        {0x64, 1, "LD H, H",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::H},
        {0x65, 1, "LD H, L",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::L},
        {0x66, 1, "LD H, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::H,    Register::HL},
        {0x67, 1, "LD H, A",           Instruction::LD,   AddressMode::R_R,    Register::H,    Register::A},
        {0x68, 1, "LD L, B",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::B},
        {0x69, 1, "LD L, C",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::C},
        {0x6A, 1, "LD L, D",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::D},
        {0x6B, 1, "LD L, E",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::E},
        {0x6C, 1, "LD L, H",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::H},
        {0x6D, 1, "LD L, L",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::L},
        {0x6E, 1, "LD L, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::L,    Register::HL},
        {0x6F, 1, "LD L, A",           Instruction::LD,   AddressMode::R_R,    Register::L,    Register::A},

        //0x7X
        {0x70, 1, "LD (HL), B",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::B},
        {0x71, 1, "LD (HL), C",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::C},
        {0x72, 1, "LD (HL), D",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::D},
        {0x73, 1, "LD (HL), E",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::E},
        {0x74, 1, "LD (HL), H",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::H},
        {0x75, 1, "LD (HL), L",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::L},
        {0x76, 1, "HALT",              Instruction::HALT},
        {0x77, 1, "LD (HL), A",        Instruction::LD,   AddressMode::MR_R,   Register::HL,   Register::A},
        {0x78, 1, "LD A, B",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::B},
        {0x79, 1, "LD A, C",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::C},
        {0x7A, 1, "LD A, D",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::D},
        {0x7B, 1, "LD A, E",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::E},
        {0x7C, 1, "LD A, H",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::H},
        {0x7D, 1, "LD A, L",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::L},
        {0x7E, 1, "LD A, (HL)",        Instruction::LD,   AddressMode::R_MR,   Register::A,    Register::HL},
        {0x7F, 1, "LD A, A",           Instruction::LD,   AddressMode::R_R,    Register::A,    Register::A},

        //0x8X
        {0x80, 1, "ADD A, B",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::B},
        {0x81, 1, "ADD A, C",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::C},
        {0x82, 1, "ADD A, D",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::D},
        {0x83, 1, "ADD A, E",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::E},
        {0x84, 1, "ADD A, H",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::H},
        {0x85, 1, "ADD A, L",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::L},
        {0x86, 1, "ADD A, (HL)",       Instruction::ADD,  AddressMode::R_MR,   Register::A,    Register::HL},
        {0x87, 1, "ADD A, A",          Instruction::ADD,  AddressMode::R_R,    Register::A,    Register::A},
        {0x88, 1, "ADC A, B",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::B},
        {0x89, 1, "ADC A, C",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::C},
        {0x8A, 1, "ADC A, D",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::D},
        {0x8B, 1, "ADC A, E",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::E},
        {0x8C, 1, "ADC A, H",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::H},
        {0x8D, 1, "ADC A, L",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::L},
        {0x8E, 1, "ADC A, (HL)",       Instruction::ADC,  AddressMode::R_MR,   Register::A,    Register::HL},
        {0x8F, 1, "ADC A, A",          Instruction::ADC,  AddressMode::R_R,    Register::A,    Register::A},

        //0x9X
        {0x90, 1, "SUB A, B",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::B},
        {0x91, 1, "SUB A, C",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::C},
        {0x92, 1, "SUB A, D",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::D},
        {0x93, 1, "SUB A, E",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::E},
        {0x94, 1, "SUB A, H",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::H},
        {0x95, 1, "SUB A, L",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::L},
        {0x96, 1, "SUB A, (HL)",       Instruction::SUB,  AddressMode::R_MR,   Register::A,    Register::HL},
        {0x97, 1, "SUB A, A",          Instruction::SUB,  AddressMode::R_R,    Register::A,    Register::A},
        {0x98, 1, "SBC A, B",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::B},
        {0x99, 1, "SBC A, C",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::C},
        {0x9A, 1, "SBC A, D",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::D},
        {0x9B, 1, "SBC A, E",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::E},
        {0x9C, 1, "SBC A, H",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::H},
        {0x9D, 1, "SBC A, L",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::L},
        {0x9E, 1, "SBC A, (HL)",       Instruction::SBC,  AddressMode::R_MR,   Register::A,    Register::HL},
        {0x9F, 1, "SBC A, A",          Instruction::SBC,  AddressMode::R_R,    Register::A,    Register::A},


        //0xAX
        {0xA0, 1, "AND A, B",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::B},
        {0xA1, 1, "AND A, C",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::C},
        {0xA2, 1, "AND A, D",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::D},
        {0xA3, 1, "AND A, E",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::E},
        {0xA4, 1, "AND A, H",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::H},
        {0xA5, 1, "AND A, L",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::L},
        {0xA6, 1, "AND A, (HL)",       Instruction::AND,  AddressMode::R_MR,   Register::A,    Register::HL},
        {0xA7, 1, "AND A, A",          Instruction::AND,  AddressMode::R_R,    Register::A,    Register::A},
        {0xA8, 1, "XOR A, B",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::B},
        {0xA9, 1, "XOR A, C",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::C},
        {0xAA, 1, "XOR A, D",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::D},
        {0xAB, 1, "XOR A, E",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::E},
        {0xAC, 1, "XOR A, H",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::H},
        {0xAD, 1, "XOR A, L",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::L},
        {0xAE, 1, "XOR A, (HL)",       Instruction::XOR,  AddressMode::R_MR,   Register::A,    Register::HL},
        {0xAF, 1, "XOR A, A",          Instruction::XOR,  AddressMode::R_R,    Register::A,    Register::A},

        //0xBX
        {0xB0, 1, "OR A, B",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::B},
        {0xB1, 1, "OR A, C",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::C},
        {0xB2, 1, "OR A, D",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::D},
        {0xB3, 1, "OR A, E",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::E},
        {0xB4, 1, "OR A, H",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::H},
        {0xB5, 1, "OR A, L",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::L},
        {0xB6, 1, "OR A, (HL)",        Instruction::OR,   AddressMode::R_MR,   Register::A,    Register::HL},
        {0xB7, 1, "OR A, A",           Instruction::OR,   AddressMode::R_R,    Register::A,    Register::A},
        {0xB8, 1, "CP A, B",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::B},
        {0xB9, 1, "CP A, C",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::C},
        {0xBA, 1, "CP A, D",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::D},
        {0xBB, 1, "CP A, E",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::E},
        {0xBC, 1, "CP A, H",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::H},
        {0xBD, 1, "CP A, L",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::L},
        {0xBE, 1, "CP A, (HL)",        Instruction::CP,   AddressMode::R_MR,   Register::A,    Register::HL},
        {0xBF, 1, "CP A, A",           Instruction::CP,   AddressMode::R_R,    Register::A,    Register::A},

        //0xC0
        {0xC0, 1, "RET NZ",            Instruction::RET,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NZ},
        {0xC1, 1, "POP BC",            Instruction::POP,  AddressMode::R,      Register::BC},
        {0xC2, 3, "JP NZ, u16",        Instruction::JP,   AddressMode::D16,    Register::NONE, Register::NONE, Condition::NZ},
        {0xC3, 3, "JP u16",            Instruction::JP,   AddressMode::D16},
        {0xC4, 3, "CALL NZ, u16",      Instruction::CALL, AddressMode::D16,    Register::NONE, Register::NONE, Condition::NZ},
        {0xC5, 1, "PUSH BC",           Instruction::PUSH, AddressMode::R,      Register::BC},
        {0xC6, 2, "ADD A, u8",         Instruction::ADD,  AddressMode::R_D8,   Register::A},
        {0xC7, 1, "RST 00h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x00},
        {0xC8, 1, "RET Z",             Instruction::RET,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::Z},
        {0xC9, 1, "RET",               Instruction::RET},
        {0xCA, 3, "JP Z, u16",         Instruction::JP,   AddressMode::D16,    Register::NONE, Register::NONE, Condition::Z},
        {0xCB, 2, "<CB>",              Instruction::CB,   AddressMode::D8},
        {0xCC, 3, "CALL Z, u16",       Instruction::CALL, AddressMode::D16,    Register::NONE, Register::NONE, Condition::Z},
        {0xCD, 3, "CALL u16",          Instruction::CALL, AddressMode::D16},
        {0xCE, 2, "ADC A, u8",         Instruction::ADC,  AddressMode::R_D8,   Register::A},
        {0xCF, 1, "RST 08h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x08},

        // 0xD0
        {0xD0, 1, "RET NC",            Instruction::RET,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NC},
        {0xD1, 1, "POP DE",            Instruction::POP,  AddressMode::R,      Register::DE},
        {0xD2, 3, "JP NC, u16",        Instruction::JP,   AddressMode::D16,    Register::NONE, Register::NONE, Condition::NC},
        {0xD3, 1, "NOP D3",            Instruction::NOP,  AddressMode::IMP},
        {0xD4, 3, "CALL NC, u16",      Instruction::CALL, AddressMode::D16,    Register::NONE, Register::NONE, Condition::NC},
        {0xD5, 1, "PUSH DE",           Instruction::PUSH, AddressMode::R,      Register::DE},
        {0xD6, 2, "SUB A, u8",         Instruction::SUB,  AddressMode::R_D8,   Register::A},
        {0xD7, 1, "RST 10h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x10},
        {0xD8, 1, "RET C",             Instruction::RET,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::C},
        {0xD9, 1, "RETI",              Instruction::RETI},
        {0xDA, 3, "JP C, u16",         Instruction::JP,   AddressMode::D16,    Register::NONE, Register::NONE, Condition::C},
        {0xDB, 1, "NOP DB",            Instruction::NOP,  AddressMode::IMP},
        {0xDC, 3, "CALL C, u16",       Instruction::CALL, AddressMode::D16,    Register::NONE, Register::NONE, Condition::C},
        {0xDD, 1, "NOP DD",            Instruction::NOP,  AddressMode::IMP},
        {0xDE, 2, "SBC A, u8",         Instruction::SBC,  AddressMode::R_D8,   Register::A},
        {0xDF, 1, "RST 18h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x18},

        //0xEX
        {0xE0, 2, "LD (FF00 + u8), A", Instruction::LDH,  AddressMode::A8_R,   Register::NONE, Register::A},
        {0xE1, 1, "POP HL",            Instruction::POP,  AddressMode::R,      Register::HL},
        {0xE2, 1, "LD (FF00 + C), A",  Instruction::LD,   AddressMode::MR_R,   Register::C,    Register::A},
        {0xE3, 1, "NOP E3",            Instruction::NOP,  AddressMode::IMP},
        {0xE4, 1, "NOP E4",            Instruction::NOP,  AddressMode::IMP},
        {0xE5, 1, "PUSH HL",           Instruction::PUSH, AddressMode::R,      Register::HL},
        {0xE6, 2, "AND A, u8",         Instruction::AND,  AddressMode::R_D8,   Register::A},
        {0xE7, 1, "RST 20h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x20},
        {0xE8, 2, "ADD SP, i8",        Instruction::ADD,  AddressMode::R_D8,   Register::SP},
        {0xE9, 1, "JP HL",             Instruction::JP,   AddressMode::R,      Register::HL},
        {0xEA, 3, "LD (u16), A",       Instruction::LD,   AddressMode::A16_R,  Register::NONE, Register::A},
        {0xEB, 1, "NOP EB",            Instruction::NOP,  AddressMode::IMP},
        {0xEC, 1, "NOP EC",            Instruction::NOP,  AddressMode::IMP},
        {0xED, 1, "NOP ED",            Instruction::NOP,  AddressMode::IMP},
        {0xEE, 2, "XOR A, u8",         Instruction::XOR,  AddressMode::R_D8,   Register::A},
        {0xEF, 1, "RST 28h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x28},


        //0xFX
        {0xF0, 2, "LD A, (FF00 + u8)", Instruction::LDH,  AddressMode::R_A8,   Register::A},
        {0xF1, 1, "POP AF",            Instruction::POP,  AddressMode::R,      Register::AF},
        {0xF2, 1, "LD A, (FF00 + C)",  Instruction::LD,   AddressMode::R_MR,   Register::A,    Register::C},
        {0xF3, 1, "DI",                Instruction::DI},
        {0xF4, 1, "NOP F4",            Instruction::NOP,  AddressMode::IMP},
        {0xF5, 1, "PUSH AF",           Instruction::PUSH, AddressMode::R,      Register::AF},
        {0xF6, 2, "OR A, u8",          Instruction::OR,   AddressMode::R_D8,   Register::A},
        {0xF7, 1, "RST 30h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x30},
        {0xF8, 2, "LD HL, SP + i8",    Instruction::LD,   AddressMode::HL_SPR, Register::HL,   Register::SP},
        {0xF9, 1, "LD SP, HL",         Instruction::LD,   AddressMode::R_R,    Register::SP,   Register::HL},
        {0xFA, 3, "LD A, (u16)",       Instruction::LD,   AddressMode::R_A16,  Register::A},
        {0xFB, 1, "EI",                Instruction::EI},
        {0xFC, 1, "NOP FC",            Instruction::NOP,  AddressMode::IMP},
        {0xFD, 1, "NOP FD",            Instruction::NOP,  AddressMode::IMP},
        {0xFE, 2, "CP A, u8",          Instruction::CP,   AddressMode::R_D8,   Register::A},
        {0xFF, 1, "RST 38h",           Instruction::RST,  AddressMode::IMP,    Register::NONE, Register::NONE, Condition::NONE, 0x38},
    };

    std::unordered_map<AddressMode, std::string> m_AddressModeNames{
        {AddressMode::IMP,    "IMP"},
        {AddressMode::R_D16,  "EXTI"},
        {AddressMode::R_R,    "REG"},
        {AddressMode::MR_R,   "REGI"},
        {AddressMode::R,      "REG"},
        {AddressMode::R_D8,   "IMM"},
        {AddressMode::R_MR,   "REGI"},
        {AddressMode::R_HLI,  "REGI"},
        {AddressMode::R_HLD,  "REGI"},
        {AddressMode::HLI_R,  "REGI"},
        {AddressMode::HLD_R,  "REGI"},
        {AddressMode::R_A8,   "REL"},
        {AddressMode::A8_R,   "RELH"},
        {AddressMode::HL_SPR, "REG"},
        {AddressMode::D16,    "EXTI"},
        {AddressMode::D8,     "IMM"},
        {AddressMode::D16_R,  "REGI"},
        {AddressMode::MR_D8,  "REGI"},
        {AddressMode::MR,     "REGI"},
        {AddressMode::A16_R,  "EXT"},
        {AddressMode::R_A16,  "EXT"}
    };

    std::unordered_map<Register, std::string> m_RegisterNames{
        {Register::NONE, "NONE"},
        {Register::A,    "A"},
        {Register::B,    "B"},
        {Register::C,    "C"},
        {Register::D,    "D"},
        {Register::E,    "E"},
        {Register::H,    "H"},
        {Register::L,    "L"},
        {Register::AF,   "AF"},
        {Register::BC,   "BC"},
        {Register::DE,   "DE"},
        {Register::HL,   "HL"},
        {Register::SP,   "SP"},
        {Register::PC,   "PC"}
    };

  };

} // hijo
