#include "SharpSM83.h"

namespace hijo {
  SharpSM83::SharpSM83() {
    Reset();
    //InitCBOps();
    //InitOpcodes();
  }

  void SharpSM83::InitCBOps() {
    m_CBOps = {
        {0x0, "RLC B", 8,
         [this]() {
           RLC(Register::B);
           return -8;
         }
        }
    };
  }

  void SharpSM83::InitOpcodes() {
    m_Opcodes = {
        {0x0,  "NOP",               AddressingMode::Implied,            1, 4,
            []() {
              return 0;
            }
        },
        {0x1,  "LD BC, u16",        AddressingMode::ExtendedImmediate,  3, 12,
            [this]() {
              LD(Register::BC, latch);
              return 0;
            }
        },
        {0x2,  "LD (BC), A",        AddressingMode::RegisterIndirectBC, 1, 8,
            [this]() {
              LD(latch, regs.a);
              return 0;
            }},
        {0x3,  "INC BC",            AddressingMode::Implied,            1, 8,
            [this]() {
              INC(Register::BC);
              return 0;
            }
        },
        {0x4,  "INC B",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::B);
              return 0;
            }
        },
        {0x5,  "DEC B",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::B);
              return 0;
            }
        },
        {0x6,  "LD B, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::B,
                 static_cast<uint8_t>(latch & 0xFF));

              return 0;
            }
        },
        {0x7,  "RLCA",              AddressingMode::Implied,            1, 4,
            [this]() {
              uint16_t res =
                  (regs.a << 1) | Carry();
              ClearZero();
              ClearNegative();
              ClearHalfCarry();
              SetCarry(res);
              regs.a = res & 0xFF;

              return 0;
            }
        },
        {0x8,  "LD (u16), SP",      AddressingMode::Extended,           3, 20,
            [this]() {
              LD(latch, regs.sp);

              return 0;
            }
        },
        {0x9,  "ADD HL, BC",        AddressingMode::Implied,            1, 8,
            [this]() {
              ADD(Register::HL, Register::BC, true);
              return 0;
            },
        },
        {0xA,  "LD A, (BC)",        AddressingMode::RegisterIndirectBC, 1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(latch));
              return 0;
            }
        },
        {0xB,  "DEC BC",            AddressingMode::Implied,            1, 8,
            [this]() {
              DEC(Register::BC);
              return 0;
            }
        },
        {0xC,  "INC C",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::C);

              return 0;
            }
        },
        {0xD,  "DEC C",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::C);

              return 0;
            }
        },
        {0xE,  "LD C, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::C,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xF,  "RRCA",              AddressingMode::Implied,            1, 4,
            [this]() {
              uint8_t low = regs.a & 1;
              uint16_t res =
                  (regs.a >> 1) | (low << 7);

              ClearZero();
              ClearHalfCarry();
              ClearNegative();
              if (low) {
                SetCarry();
              } else {
                ClearCarry();
              }

              regs.c = res & 0xFF;

              return 0;
            }
        },
        {0x10, "STOP",              AddressingMode::Implied,            1, 4,
            [this]() {
              m_Stopped = true;
              return 0;
            }
        },
        {0x11, "LD DE, u16",        AddressingMode::ExtendedImmediate,  3, 12,
            [this]() {
              LD(Register::DE, latch);
              return 0;
            }},
        {0x12, "LD (DE), A",        AddressingMode::RegisterIndirectDE, 1, 8,
            [this]() {

              LD(regs.de, regs.a);
              return 0;
            }
        },
        {0x13, "INC DE",            AddressingMode::Implied,            1, 8,
            [this]() {
              DEC(Register::DE);

              return 0;
            }
        },
        {0x14, "INC D",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::D);
              return 0;
            }
        },
        {0x15, "DEC D",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::D);

              return 0;
            }
        },
        {0x16, "LD D, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::D,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0x17, "RLA",               AddressingMode::Implied,            1, 4,
            [this]() {
              int wide =
                  (regs.a << 1) | Carry();

              ClearNegative();
              ClearHalfCarry();
              ClearZero();
              SetCarry(wide);

              return 0;
            }
        },
        {0x18, "JR i8",             AddressingMode::Relative,           2, 12,
            [this]() {
              JR(static_cast<int8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0x19, "ADD HL, DE",        AddressingMode::Implied,            1, 8,
            [this]() {
              ADD(Register::HL, Register::DE, true);

              return 0;
            },
        },
        {0x1A, "LD A, (DE)",        AddressingMode::RegisterIndirectDE, 1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.de));

              return 0;
            },
        },
        {0x1B, "DEC DE",            AddressingMode::Implied,            1, 8,
            [this]() {
              DEC(Register::DE);

              return 0;
            },
        },
        {0x1C, "INC E",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::E);

              return 0;
            },
        },
        {0x1D, "DEC E",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::E);

              return 0;
            },
        },
        {0x1E, "LD E, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::E,
                 static_cast<uint8_t>(latch & 0xFF));

              return 0;
            },
        },
        {0x1F, "RRA",               AddressingMode::Implied,            1, 4,
            [this]() {
              uint8_t low = regs.a & 1;
              regs.a =
                  ((regs.a >> 1) | Carry()) << 7;

              ClearZero();
              ClearNegative();
              ClearHalfCarry();

              if (low) {
                SetCarry();
              } else {
                ClearCarry();
              }

              return 0;
            },
        },
        {0x20, "JR NZ, i8",         AddressingMode::Relative,           2, 12,
            [this]() {

              if (Zero() == 0)
                return -4;

              JR(static_cast<int8_t>(latch & 0xFF));
              return 0;
            },
        },
        {0x21, "LD HL, u16",        AddressingMode::ExtendedImmediate,  3, 12,
            [this]() {
              LD(Register::HL, latch);
              return 0;
            }
        },
        {0x22, "LD (HL+), A",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(regs.hl++, regs.a);

              return 0;
            }
        },
        {0x23, "INC HL",            AddressingMode::Implied,            1, 8,
            [this]() {
              INC(Register::HL);

              return 0;
            }
        },
        {0x24, "INC H",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::H);

              return 0;
            }
        },
        {0x25, "DEC H",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::H);

              return 0;
            }
        },
        {0x26, "LD H, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::H,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0x27, "DAA",               AddressingMode::Implied,            1, 4,
            [this]() {
              if (Negative() != 0) {
                if (HalfCarry() != 0) {
                  regs.a += 0xFA;
                }
                if (Carry()) {
                  regs.a += 0xA0;
                }
              } else {
                uint32_t a = regs.a;

                if ((regs.a & 0xF) > 0x9 ||
                    HalfCarry()) {
                  a += 0x6;
                }

                if ((a & 0x1F0) > 0x90 ||
                    Carry()) {
                  a += 0x60;
                  SetCarry();
                } else {
                  ClearCarry();
                }

                regs.a = (uint8_t) (a & 0xFF);
              }

              ClearHalfCarry();
              SetZero(!regs.a);
              return 0;
            }
        },
        {0x28, "JR Z, i8",          AddressingMode::Relative,           2, 12,
            [this]() {
              if (Zero() != 0)
                return -4;

              JR(static_cast<int8_t>(latch & 0xFF));
              return 0;
            },
        },
        {0x29, "ADD HL, HL",        AddressingMode::Implied,            1, 8,
            [this]() {
              ADD(Register::HL, Register::HL);
              return 0;
            },
        },
        {0x2A, "LD A, (HL+)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.hl++));
              return 0;
            },
        },
        {0x2B, "DEC HL",            AddressingMode::Implied,            1, 8,
            [this]() {
              DEC(Register::HL);
              return 0;
            },
        },
        {0x2C, "INC L",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::L);
              return 0;
            },
        },
        {0x2D, "DEC L",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::L);
              return 0;
            },
        },
        {0x2E, "LD L, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::L,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            },
        },

        {0x2F, "CPL",               AddressingMode::Implied,            1, 4,
            [this]() {
              regs.a ^= 0xFF;
              SetHalfCarry();
              SetNegative();
              return 0;
            },
        },
        {0x30, "JR NC, i8",         AddressingMode::Relative,           2, 12,
            [this]() {
              if (Carry())
                return -4;

              JR(static_cast<int8_t>(latch & 0xFF));
              return 0;
            },
        },
        {0x31, "LD SP, u16",        AddressingMode::ExtendedImmediate,  3, 12,
            [this]() {
              LD(Register::SP, latch);
              return 0;
            }
        },
        {0x32, "LD (HL-), A",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(regs.hl--, regs.a);
              return 0;
            }
        },
        {0x33, "INC SP",            AddressingMode::Implied,            1, 8,
            [this]() {
              INC(Register::SP);
              return 0;
            }
        },
        {0x34, "INC (HL)",          AddressingMode::RegisterIndirectHL, 1, 12,
            [this]() {
              INC(regs.hl);
              return 0;
            }
        },
        {0x35, "DEC (HL)",          AddressingMode::RegisterIndirectHL, 1, 12,
            [this]() {
              DEC(regs.hl);
              return 0;
            }
        },
        {0x36, "LD (HL), u8",       AddressingMode::RegisterIndirectHL, 2, 12,
            [this]() {
              LD(regs.hl,
                 static_cast<uint8_t>(latch & 0xFF));

              return 0;
            }
        },
        {0x37, "SCF",               AddressingMode::Implied,            1, 4,
            [this]() {
              ClearNegative();
              ClearHalfCarry();
              SetCarry();
              return 0;
            }
        },
        {0x38, "JR C, i8",          AddressingMode::Relative,           2, 12,
            [this]() {

              if (Carry() == 0)
                return -4;

              JR(static_cast<int8_t>(latch & 0xFF));
              return 0;
            },
        },
        {0x39, "ADD HL, SP",        AddressingMode::Implied,            1, 8,
            [this]() {
              ADD(Register::HL, Register::SP, true);
              return 0;
            },
        },
        {0x3A, "LD A, (HL-)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.hl--));
              return 0;
            },
        },
        {0x3B, "DEC SP",            AddressingMode::Implied,            1, 8,
            [this]() {
              DEC(Register::SP);
              return 0;
            },
        },
        {0x3C, "INC A",             AddressingMode::Register,           1, 4,
            [this]() {
              INC(Register::A);
              return 0;
            },
        },
        {0x3D, "DEC A",             AddressingMode::Register,           1, 4,
            [this]() {
              DEC(Register::A);
              return 0;
            },
        },
        {0x3E, "LD A, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              LD(Register::A,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            },
        },
        {0x3F, "CCF",               AddressingMode::Implied,            1, 4,
            [this]() {
              ClearNegative();
              ClearHalfCarry();

              if (Carry()) {
                ClearCarry();
              } else {
                SetCarry();
              }

              return 0;
            }
        },
        {0x40, "LD B, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::B);
              return 0;
            }
        },
        {0x41, "LD B, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::C);
              return 0;
            }
        },
        {0x42, "LD B, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::D);
              return 0;
            }
        },
        {0x43, "LD B, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::E);
              return 0;
            }
        },
        {0x44, "LD B, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::H);
              return 0;
            }
        },
        {0x45, "LD B, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::L);
              return 0;
            }
        },
        {0x46, "LD B, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::B, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x47, "LD B, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::B, Register::A);
              return 0;
            }
        },
        {0x48, "LD C, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::B);
              return 0;
            }
        },
        {0x49, "LD C, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::C);
              return 0;
            }
        },
        {0x4A, "LD C, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::D);
              return 0;
            }
        },
        {0x4B, "LD C, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::E);
              return 0;
            }
        },
        {0x4C, "LD C, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::H);
              return 0;
            }
        },
        {0x4D, "LD C, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::L);
              return 0;
            }
        },
        {0x4E, "LD C, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::C, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x4F, "LD C, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::C, Register::A);
              return 0;
            }
        },
        {0x50, "LD D, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::B);
              return 0;
            }
        },
        {0x51, "LD D, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::C);
              return 0;
            }
        },
        {0x52, "LD D, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::D);
              return 0;
            }
        },
        {0x53, "LD D, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::E);
              return 0;
            }
        },
        {0x54, "LD D, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::H);
              return 0;
            }
        },
        {0x55, "LD D, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::L);
              return 0;
            }
        },
        {0x56, "LD D, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::D, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x57, "LD D, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::D, Register::A);
              return 0;
            }
        },
        {0x58, "LD E, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::B);
              return 0;
            }
        },
        {0x59, "LD E, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::C);
              return 0;
            }
        },
        {0x5A, "LD E, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::D);
              return 0;
            }
        },
        {0x5B, "LD E, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::E);
              return 0;
            }
        },
        {0x5C, "LD E, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::H);
              return 0;
            }
        },
        {0x5D, "LD E, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::L);
              return 0;
            }
        },
        {0x5E, "LD E, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::E, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x5F, "LD E, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::E, Register::A);
              return 0;
            }
        },
        {0x60, "LD H, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::B);
              return 0;
            }
        },
        {0x61, "LD H, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::C);
              return 0;
            }
        },
        {0x62, "LD H, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::D);
              return 0;
            }
        },
        {0x63, "LD H, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::E);
              return 0;
            }
        },
        {0x64, "LD H, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::H);
              return 0;
            }
        },
        {0x65, "LD H, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::L);
              return 0;
            }
        },
        {0x66, "LD H, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::H, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x67, "LD H, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::H, Register::A);
              return 0;
            }
        },
        {0x68, "LD L, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::B);
              return 0;
            }
        },
        {0x69, "LD L, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::C);
              return 0;
            }
        },
        {0x6A, "LD L, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::D);
              return 0;
            }
        },
        {0x6B, "LD L, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::E);
              return 0;
            }
        },
        {0x6C, "LD L, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::H);
              return 0;
            }
        },
        {0x6D, "LD L, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::L);
              return 0;
            }
        },
        {0x6E, "LD L, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::L, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x6F, "LD L, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::L, Register::A);
              return 0;
            }
        },
        {
         0x70, "LD (HL), B",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.b);
              return 0;
            }
        },
        {
         0x71, "LD (HL), C",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.c);
              return 0;
            }
        },
        {0x72, "LD (HL), D",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.d);
              return 0;
            }
        },
        {0x73, "LD (HL), E",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.e);
              return 0;
            }
        },
        {0x74, "LD (HL), H",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.h);
              return 0;
            }
        },
        {0x75, "LD (HL), L",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.l);
              return 0;
            }
        },
        {0x76, "HALT",              AddressingMode::Implied,            1, 4,
            [this]() {
              m_Halted = true;
              return 0;
            }
        },
        {0x77, "LD (HL), A",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(latch, regs.a);
              return 0;
            }
        },
        {0x78, "LD A, B",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::B);
              return 0;
            }
        },
        {0x79, "LD A, C",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::C);
              return 0;
            }
        },
        {0x7A, "LD A, D",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::D);
              return 0;
            }
        },
        {0x7B, "LD A, E",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::E);
              return 0;
            }
        },
        {0x7C, "LD A, H",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::H);
              return 0;
            }
        },
        {0x7D, "LD A, L",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::L);
              return 0;
            }
        },
        {0x7E, "LD A, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x7F, "LD A, A",           AddressingMode::Register,           1, 4,
            [this]() {
              LD(Register::A, Register::A);
              return 0;
            }
        },
        {0x80, "ADD A, B",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::B);
              return 0;
            }
        },
        {0x81, "ADD A, C",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::C);
              return 0;
            }
        },
        {0x82, "ADD A, D",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::D);
              return 0;
            }
        },
        {0x83, "ADD A, E",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::E);
              return 0;
            }
        },
        {0x84, "ADD A, H",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::H);
              return 0;
            }
        },
        {0x85, "ADD A, L",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::L);
              return 0;
            }
        },
        {0x86, "ADD A, (HL)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              ADD(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x87, "ADD A, A",          AddressingMode::Register,           1, 4,
            [this]() {
              ADD(Register::A, Register::A);
              return 0;
            }
        },
        {0x87, "ADC A, B",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::B);
              return 0;
            }
        },
        {0x88, "ADC A, C",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::C);
              return 0;
            }
        },
        {0x8A, "ADC A, D",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::D);
              return 0;
            }
        },
        {0x8B, "ADC A, E",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::E);
              return 0;
            }
        },
        {0x8C, "ADC A, H",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::H);
              return 0;
            }
        },
        {0x8D, "ADC A, L",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::L);
              return 0;
            }
        },
        {0x8E, "ADC A, (HL)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              ADC(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x8F, "ADC A, A",          AddressingMode::Register,           1, 4,
            [this]() {
              ADC(Register::A);
              return 0;
            }
        },
        {0x90, "SUB A, B",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::B);
              return 0;
            }
        },
        {0x91, "SUB A, C",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::C);
              return 0;
            }
        },
        {0x92, "SUB A, D",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::D);
              return 0;
            }
        },
        {0x93, "SUB A, E",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::E);
              return 0;
            }
        },
        {0x94, "SUB A, H",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::H);
              return 0;
            }
        },
        {0x95, "SUB A, L",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::L);
              return 0;
            }
        },
        {0x96, "SUB A, (HL)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              SUB(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x97, "SUB A, A",          AddressingMode::Register,           1, 4,
            [this]() {
              SUB(Register::A);
              return 0;
            }
        },
        {0x98, "SBC A, B",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::B);
              return 0;
            }
        },
        {0x99, "SBC A, C",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::C);
              return 0;
            }
        },
        {0x9A, "SBC A, D",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::D);
              return 0;
            }
        },
        {0x9B, "SBC A, E",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::E);
              return 0;
            }
        },
        {0x9C, "SBC A, H",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::H);
              return 0;
            }
        },
        {0x9D, "SBC A, L",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::L);
              return 0;
            }
        },
        {0x9E, "SBC A, (HL)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              SBC(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0x9F, "SBC A, A",          AddressingMode::Register,           1, 4,
            [this]() {
              SBC(Register::A);
              return 0;
            }
        },
        {0xA0, "AND A, B",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::B);
              return 0;
            }
        },
        {0xA1, "AND A, C",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::C);
              return 0;
            }
        },
        {0xA2, "AND A, D",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::D);
              return 0;
            }
        },
        {0xA3, "AND A, E",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::E);
              return 0;
            }
        },
        {0xA4, "AND A, H",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::H);
              return 0;
            }
        },
        {0xA5, "AND A, L",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::L);
              return 0;
            }
        },
        {0xA6, "AND A, (HL)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              AND(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0xA7, "AND A, A",          AddressingMode::Register,           1, 4,
            [this]() {
              AND(Register::A);
              return 0;
            }
        },
        {0xA8, "XOR A, B",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::B);
              return 0;
            }
        },
        {0xA9, "XOR A, C",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::C);
              return 0;
            }
        },
        {0xAA, "XOR A, D",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::D);
              return 0;
            }
        },
        {0xAB, "XOR A, E",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::E);
              return 0;
            }
        },
        {0xAC, "XOR A, H",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::H);
              return 0;
            }
        },
        {0xAD, "XOR A, L",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::L);
              return 0;
            }
        },
        {0xAE, "XOR A, (HL)",       AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              XOR(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0xAF, "XOR A, A",          AddressingMode::Register,           1, 4,
            [this]() {
              XOR(Register::A);
              return 0;
            }
        },
        {0xB0, "OR A, B",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::B);
              return 0;
            }
        },
        {0xB1, "OR A, C",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::C);
              return 0;
            }
        },
        {0xB2, "OR A, D",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::D);
              return 0;
            }
        },
        {0xB3, "OR A, E",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::E);
              return 0;
            }
        },
        {0xB4, "OR A, H",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::H);
              return 0;
            }
        },
        {0xB5, "OR A, L",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::L);
              return 0;
            }
        },
        {0xB6, "OR A, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              OR(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0xB7, "OR A, A",           AddressingMode::Register,           1, 4,
            [this]() {
              OR(Register::A);
              return 0;
            }
        },
        {0xB8, "CP A, B",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::B);
              return 0;
            }
        },
        {0xB9, "CP A, C",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::C);
              return 0;
            }
        },
        {0xBA, "CP A, D",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::D);
              return 0;
            }
        },
        {0xBB, "CP A, E",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::E);
              return 0;
            }
        },
        {0xBC, "CP A, H",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::H);
              return 0;
            }
        },
        {0xBD, "CP A, L",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::L);
              return 0;
            }
        },
        {0xBE, "CP A, (HL)",        AddressingMode::RegisterIndirectHL, 1, 8,
            [this]() {
              CP(bus->cpuRead(regs.hl));
              return 0;
            }
        },
        {0xBF, "CP A, A",           AddressingMode::Register,           1, 4,
            [this]() {
              CP(Register::A);
              return 0;
            }
        },
        {0xC0, "RET NZ",            AddressingMode::Implied,            1, 20,
            [this]() {
              if (Zero() == 0)
                return -12;
              RET();
              return 0;
            }
        },
        {0xC1, "POP BC",            AddressingMode::Implied,            1, 12,
            [this]() {
              POP(Register::BC);
              return 0;
            }
        },
        {0xC2, "JP NZ, u16",        AddressingMode::Extended,           3, 16,
            [this]() {
              if (Zero() == 0)
                return -4;

              JP(latch);
              return 0;
            }
        },
        {0xC3, "JP u16",            AddressingMode::Extended,           3, 16,
            [this]() {
              JP(latch);
              return 0;
            }
        },
        {0xC4, "CALL NZ, u16",      AddressingMode::Extended,           3, 24,
            [this]() {
              if (Zero() == 0)
                return -12;

              CALL(latch);
              return 0;
            }
        },
        {0xC5, "PUSH BC",           AddressingMode::Implied,            1, 16,
            [this]() {
              PUSH(Register::BC);
              return 0;
            }
        },
        {0xC6, "ADD A, u8",         AddressingMode::Immediate,          2, 8,
            [this]() {
              ADD(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xC7, "RST 00h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0);
              return 0;
            }
        },
        {0xC8, "RET Z",             AddressingMode::Implied,            1, 20,
            [this]() {
              if (Zero() != 0)
                return -12;
              RET();
              return 0;
            }
        },
        {0xC9, "RET",               AddressingMode::Implied,            1, 16,
            [this]() {
              RET();
              return 0;
            }
        },
        {0xCA, "JP Z, u16",         AddressingMode::Extended,           3, 16,
            [this]() {
              if (Zero() != 0)
                return -4;

              JP(latch);
              return 0;
            }
        },
        {0xCB, "PREFIX CB",         AddressingMode::Immediate,          2, 16,
            [this]() {
              size_t index = latch & 0xFF;
              auto &op = m_CBOps[index];

              return op.exec();
            }
        },
        {0xCC, "CALL Z, u16",       AddressingMode::Extended,           3, 24,
            [this]() {
              if (Zero() != 0)
                return -12;

              CALL(latch);
              return 0;
            }
        },
        {0xCD, "CALL u16",          AddressingMode::Extended,           3, 24,
            [this]() {
              CALL(latch);
              return 0;
            }
        },
        {0xCE, "ADC A, u8",         AddressingMode::Immediate,          2, 8,
            [this]() {
              ADC(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xCF, "RST 08h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x8);
              return 0;
            }
        },
        {0xD0, "RET NC",            AddressingMode::Implied,            1, 20,
            [this]() {
              if (Zero() == 0)
                return -12;

              RET();
              return 0;
            }
        },
        {0xD1, "POP DE",            AddressingMode::Implied,            1, 12,
            [this]() {
              POP(Register::DE);
              return 0;
            }
        },
        {0xD2, "JP NC, u16",        AddressingMode::Extended,           3, 16,
            [this]() {
              if (Carry())
                return -4;

              JP(latch);
              return 0;
            }
        },
        {0xD3, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xD4, "CALL NC, u16",      AddressingMode::Extended,           3, 24,
            [this]() {
              if (Carry())
                return -12;

              CALL(latch);
              return 0;
            }
        },
        {0xD5, "PUSH DE",           AddressingMode::Implied,            1, 16,
            [this]() {
              PUSH(Register::DE);
              return 0;
            }
        },
        {0xD6, "SUB A, u8",         AddressingMode::Immediate,          2, 8,
            [this]() {
              SUB(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xD7, "RST 10h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x10);
              return 0;
            }
        },
        {0xD8, "RET C",             AddressingMode::Implied,            1, 20,
            [this]() {
              if (Carry() == 0)
                return -12;

              RET();
              return 0;
            }
        },
        {0xD9, "RETI",              AddressingMode::Implied,            1, 16,
            [this]() {
              m_InterruptsEnabled = true;
              RET();
              return 0;
            }
        },
        {0xDA, "JP C, u16",         AddressingMode::Extended,           3, 16,
            [this]() {
              if (Carry() == 0)
                return -4;

              JP(latch);
              return 0;
            }
        },
        {0xDB, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xDC, "CALL C, u16",       AddressingMode::Implied,            3, 24,
            [this]() {
              if (Carry() == 0)
                return -12;

              CALL(latch);
              return 0;
            }
        },
        {0xDD, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xDE, "SBC A, u8",         AddressingMode::Immediate,          2, 8,
            [this]() {
              SBC(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xDF, "RST 18h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x18);
              return 0;
            }
        },
        {0xE0, "LD (FF00 + u8), A", AddressingMode::Immediate,          2, 12,
            [this]() {
              LD(0xFF00 | static_cast<uint8_t>(latch & 0xFF), regs.a);
              return 0;
            }
        },
        {0xE1, "POP HL",            AddressingMode::Implied,            1, 12,
            [this]() {
              POP(Register::HL);
              return 0;
            }
        },
        {0xE2, "LD (FF00 + C), A",  AddressingMode::Implied,            1, 8,
            [this]() {
              LD(0xFF00 | regs.c, regs.a);
              return 0;
            }
        },
        {0xE3, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xE4, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xE5, "PUSH HL",           AddressingMode::Implied,            1, 16,
            [this]() {
              PUSH(Register::HL);
              return 0;
            }
        },
        {0xE6, "AND A, u8",         AddressingMode::Immediate,          2, 8,
            [this]() {
              AND(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xE7, "RST 20h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x20);
              return 0;
            }
        },
        {0xE8, "ADD SP, i8",        AddressingMode::Immediate,          2, 16,
            [this]() {
              ADD(Register::SP, static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xE9, "JP HL",             AddressingMode::Implied,            1, 4,
            [this]() {
              JP(regs.hl);
              return 0;
            }
        },
        {0xEA, "LD (u16), A",       AddressingMode::Extended,           3, 16,
            [this]() {
              LD(latch, regs.a);
              return 0;
            }
        },
        {0xEB, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xEC, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xED, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xEE, "XOR",               AddressingMode::Immediate,          2, 8,
            [this]() {
              XOR(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xEF, "RST 28h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x28);
              return 0;
            }
        },
        {0xF0, "LD A, (FF00 + u8)", AddressingMode::Immediate,          2, 12,
            [this]() {
              LD(Register::A, bus->cpuRead(0xFF00 | static_cast<uint8_t>(latch & 0xFF)));
              return 0;
            }
        },
        {0xF1, "POP AF",            AddressingMode::Implied,            1, 12,
            [this]() {
              POP(Register::AF);
              return 0;
            }
        },
        {0xF2, "LD A, (FF00 + C)",  AddressingMode::Implied,            1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(0xFF00 | regs.c));
              return 0;
            }
        },
        {0xF3, "DI",                AddressingMode::Implied,            1, 4,
            [this]() {
              m_InterruptsEnabled = false;
              return 0;
            }
        },
        {0xF4, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xF5, "PUSH AF",           AddressingMode::Implied,            1, 16,
            [this]() {
              PUSH(Register::AF);
              return 0;
            }
        },
        {0xF6, "OR A, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              OR(static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xF7, "RST 30h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x30);
              return 0;
            }
        },
        {0xF8, "LD HL, SP + i8",    AddressingMode::Immediate,          2, 12,
            [this]() {
              int32_t offset = regs.sp + static_cast<int8_t>(latch & 0xFF);

              LD(Register::HL, static_cast<uint16_t>(offset));

              ClearZero();
              ClearNegative();
              SetHalfCarry(regs.hl);
              SetCarry(regs.hl);

              return 0;
            }
        },
        {0xF9, "LD SP, HL",         AddressingMode::Implied,            1, 8,
            [this]() {
              regs.sp = regs.hl;
              return 0;
            }
        },
        {0xFA, "LD A, (u16)",       AddressingMode::Extended,           3, 16,
            [this]() {
              LD(Register::A, bus->cpuRead(latch));
              return 0;
            }
        },
        {0xFB, "EI",                AddressingMode::Implied,            1, 4,
            [this]() {
              m_InterruptsEnabled = true;
              return 0;
            }
        },
        {0xFC, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xFD, "NOP",               AddressingMode::Implied,            1, 4,
            [this]() {
              return 0;
            }
        },
        {0xFE, "CP A, u8",          AddressingMode::Immediate,          2, 8,
            [this]() {
              CP(latch & 0xFF);
              return 0;
            }
        },
        {0xFF, "RST 38h",           AddressingMode::Implied,            1, 16,
            [this]() {
              RST(0x38);
              return 0;
            }
        },
    };
  }

  void SharpSM83::ConnectBus(System *system) {
    bus = system;
  }

  void SharpSM83::Reset() {
    regs.sp = 0xfffe;
    regs.af = 0x01b0;
    regs.bc = 0x0013;
    regs.de = 0x00d8;
    regs.hl = 0x014d;
    regs.pc = 0;
  }

  void SharpSM83::LD(SharpSM83::Register r, uint16_t data) {
    switch (r) {
      case Register::A:
        regs.a = data & 0xFF;
        break;
      case Register::B:
        regs.b = data & 0xFF;
        break;
      case Register::C:
        regs.c = data & 0xFF;
        break;
      case Register::D:
        regs.d = data & 0xFF;
        break;
      case Register::E:
        regs.e = data & 0xFF;
        break;
      case Register::H:
        regs.h = data & 0xFF;
        break;
      case Register::L:
        regs.l = data & 0xFF;
        break;
      case Register::AF:
        regs.f = (data & 0xFF00) >> 8;
        regs.a = data & 0xFF;
        break;
      case Register::BC:
        regs.c = (data & 0xFF00) >> 8;
        regs.b = data & 0xFF;
        break;
      case Register::DE:
        regs.e = (data & 0xFF00) >> 8;
        regs.d = data & 0xFF;
        break;
      case Register::HL:
        regs.l = (data & 0xFF00) >> 8;
        regs.h = data & 0xFF;
        break;
      case Register::SP:
        regs.sp = data;
        break;
      case Register::PC:
        regs.pc = data;
        break;

      default:
        break;
    }
  }

  void SharpSM83::LD(SharpSM83::Register r, uint8_t data) {
    switch (r) {
      case Register::A:
        regs.a = data;
        break;
      case Register::B:
        regs.b = data;
        break;
      case Register::C:
        regs.c = data;
        break;
      case Register::D:
        regs.d = data;
        break;
      case Register::E:
        regs.e = data;
        break;
      case Register::H:
        regs.h = data;
        break;
      case Register::L:
        regs.l = data;
        break;
      case Register::AF:
        regs.af = (regs.f << 8) | data;
        break;
      case Register::BC:
        regs.bc = (regs.c << 8) | data;
        break;
      case Register::DE:
        regs.de = (regs.e << 8) | data;
        break;
      case Register::HL:
        regs.hl = (regs.l << 8) | data;
        break;
      case Register::SP:
        regs.sp = (regs.sp << 8) | data;
        break;
      case Register::PC:
        regs.pc = (regs.pc << 8) | data;
        break;

      default:
        break;
    }
  }

  void SharpSM83::LD(uint16_t address, uint8_t data) {
    bus->cpuWrite(address, data);
  }

  void SharpSM83::LD(uint16_t address, uint16_t data) {
    uint8_t low = (data & 0xFF00) >> 8;
    uint8_t high = data & 0xFF;

    LD(address, low);
    LD(address + 1, high);
  }

  void SharpSM83::LD(SharpSM83::Register target, SharpSM83::Register operand) {
    auto targetPtr = RegToPointer(target);
    auto opPtr = RegToPointer(operand);

    if (!targetPtr || !opPtr) {
      // Todo Handle this
      return;
    }

    *targetPtr = *opPtr;
  }

  void SharpSM83::INC(SharpSM83::Register r) {
    auto incReg = [this](uint8_t *r) {
      uint16_t res = *r + 1;

      SetZero(res);
      ClearNegative();
      SetHalfCarry(res);

      *r = res & 0xFF;
    };

    switch (r) {
      case Register::A:
        incReg(&regs.a);
        break;
      case Register::B:
        incReg(&regs.b);
        break;
      case Register::C:
        incReg(&regs.c);
        break;
      case Register::D:
        incReg(&regs.d);
        break;
      case Register::E:
        incReg(&regs.e);
        break;
      case Register::H:
        incReg(&regs.h);
        break;
      case Register::L:
        incReg(&regs.l);
        break;
      case Register::AF:
        regs.af++;
        break;
      case Register::BC:
        regs.bc++;
        break;
      case Register::DE:
        regs.de++;
        break;
      case Register::HL:
        regs.hl++;
        break;
      case Register::SP:
        regs.sp++;
        break;
      case Register::PC:
        regs.pc++;
        break;

      default:
        break;
    }
  }

  void SharpSM83::INC(uint16_t address) {
    auto value = bus->cpuRead(address);
    bus->cpuWrite(address, value + 1);
  }

  void SharpSM83::DEC(SharpSM83::Register r) {
    auto decReg = [this](uint8_t *r) {
      uint16_t res = *r - 1;

      SetZero(res);
      SetNegative();
      SetHalfCarry(res);

      *r = res & 0xFF;
    };

    switch (r) {
      case Register::A:
        decReg(&regs.a);
        break;
      case Register::B:
        decReg(&regs.b);
        break;
      case Register::C:
        decReg(&regs.c);
        break;
      case Register::D:
        decReg(&regs.d);
        break;
      case Register::E:
        decReg(&regs.e);
        break;
      case Register::H:
        decReg(&regs.h);
        break;
      case Register::L:
        decReg(&regs.l);
        break;
      case Register::AF:
        regs.af--;
        break;
      case Register::BC:
        regs.bc--;
        break;
      case Register::DE:
        regs.de--;
        break;
      case Register::HL:
        regs.hl--;
        break;
      case Register::SP:
        regs.sp--;
        break;
      case Register::PC:
        regs.pc--;
        break;

      default:
        break;
    }
  }

  void SharpSM83::DEC(uint16_t address) {
    auto value = bus->cpuRead(address);
    bus->cpuWrite(address, value - 1);
  }

  void SharpSM83::ADD(SharpSM83::Register target, SharpSM83::Register operand, bool isWide) {
    if (isWide) {
      auto targetPtr = WideRegToPointer(target);
      auto opPtr = WideRegToPointer(operand);

      if (!targetPtr || !opPtr) {
        // Todo Handle this
        return;
      }

      uint32_t res = *targetPtr + *opPtr;

      ClearNegative();
      SetHalfCarry(res);
      SetCarry(res);

      *targetPtr = static_cast<uint16_t>(res & 0xFFFF);
    } else {
      auto targetPtr = RegToPointer(target);
      auto opPtr = RegToPointer(operand);

      if (targetPtr != nullptr || opPtr != nullptr) {
        // Todo Handle this
        return;
      }

      uint16_t res = *targetPtr + *opPtr;

      ClearNegative();
      SetZero(res);
      SetHalfCarry(res);
      SetCarry(res);

      *targetPtr = static_cast<uint8_t>(res & 0xFF);
    }
  }

  void SharpSM83::ADD(uint8_t data) {
    uint16_t res = regs.a + data;

    SetZero(res);
    ClearNegative();
    SetHalfCarry(res);
    SetCarry(res);

    regs.a = static_cast<uint8_t>(res & 0xFF);
  }

  void SharpSM83::JR(int8_t offset) {
    int16_t res = (int16_t) regs.pc +
                  (int8_t) (offset & 0xFF);

    regs.pc = (uint16_t) res;
  }

  void SharpSM83::ADC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    ADC(*opPtr);
  }

  void SharpSM83::ADC(uint8_t data) {
    int32_t diff = regs.a + data + Carry();

    ClearNegative();
    SetHalfCarry(diff);
    SetZero(diff);
    SetCarry(diff);

    regs.a = static_cast<uint8_t>(diff);
  }

  void SharpSM83::SUB(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    SUB(*opPtr);
  }

  void SharpSM83::SUB(uint8_t data) {
    int32_t diff = regs.a - data;

    SetNegative();
    SetHalfCarry(diff);
    SetZero(diff);
    SetCarry(diff);

    regs.a = static_cast<uint8_t>(diff);
  }

  void SharpSM83::SBC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    SBC(*opPtr);
  }

  void SharpSM83::SBC(uint8_t data) {
    int32_t diff = regs.a - data - Carry();

    SetNegative();
    SetHalfCarry(diff);
    SetZero(diff);
    SetCarry(diff);

    regs.a = static_cast<uint8_t>(diff);
  }

  void SharpSM83::AND(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    AND(*opPtr);
  }

  void SharpSM83::AND(uint8_t data) {
    regs.a &= data;

    ClearNegative();
    ClearCarry();
    SetHalfCarry();
    SetZero(regs.a);
  }

  void SharpSM83::XOR(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    XOR(*opPtr);
  }

  void SharpSM83::XOR(uint8_t data) {
    regs.a ^= data;

    ClearNegative();
    ClearHalfCarry();
    ClearCarry();
    SetZero(regs.a);
  }

  void SharpSM83::OR(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    OR(*opPtr);
  }

  void SharpSM83::OR(uint8_t data) {
    regs.a |= data;

    ClearNegative();
    ClearHalfCarry();
    ClearCarry();
    SetZero(regs.a);
  }

  void SharpSM83::CP(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    CP(*opPtr);
  }

  void SharpSM83::CP(uint8_t data) {
    int32_t diff = regs.a - data;
    SetNegative();

    if (!(diff & 0xFF)) {
      SetZero();
    } else {
      ClearZero();
    }

    if ((regs.a & 0xF) - (data & 0xF) < 0) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (diff < 0)
      SetCarry();
    else
      ClearCarry();
  }

  void SharpSM83::RET() {
    regs.pc = regs.sp;
    regs.sp += 2;
  }

  void SharpSM83::POP(SharpSM83::Register operand) {
    auto targetPtr = WideRegToPointer(operand);

    if (!targetPtr) {
      // todo handle
      return;
    }

    // BC -> C, B
    // C = low, B = high

    auto low = bus->cpuRead(regs.sp++);
    auto high = bus->cpuRead(regs.sp++);

    *targetPtr = (low << 8) | high;
  }

  void SharpSM83::PUSH(SharpSM83::Register operand) {
    auto targetPtr = WideRegToPointer(operand);

    if (!targetPtr) {
      // todo handle
      return;
    }

    uint8_t low = (*targetPtr & 0xFF00) >> 8;
    uint8_t high = *targetPtr & 0xFF;

    bus->cpuWrite(--regs.sp, high);
    bus->cpuWrite(--regs.sp, low);
  }

  void SharpSM83::CALL(uint16_t address) {
    uint8_t low = (regs.pc & 0xFF00) >> 8;
    uint8_t high = regs.pc & 0xFF;

    bus->cpuWrite(--regs.sp, high);
    bus->cpuWrite(--regs.sp, low);

    regs.pc = address;
  }

  void SharpSM83::RST(uint16_t operand) {
    uint8_t low = (regs.pc & 0xFF00) >> 8;
    uint8_t high = regs.pc & 0xFF;

    bus->cpuWrite(--regs.sp, high);
    bus->cpuWrite(--regs.sp, low);

    regs.pc = operand;
  }

  void SharpSM83::RLC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RLC(*opPtr);
  }

  void SharpSM83::RLC(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RLC(data));
  }

  uint8_t SharpSM83::RLC(uint8_t data) {
    uint8_t res = (data << 1) | (data >> 7);

    ClearNegative();
    ClearHalfCarry();
    SetZero(res);
    if (res & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return res;
  }

  void SharpSM83::RRC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RRC(*opPtr);
  }


  void SharpSM83::RRC(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RRC(data));
  }

  uint8_t SharpSM83::RRC(uint8_t data) {
    uint8_t low = data & 1;
    uint8_t res = (data >> 1) | (data << 7);

    ClearNegative();
    ClearHalfCarry();
    SetZero(res);

    if (low) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return res;
  }

  void SharpSM83::RL(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RL(*opPtr);
  }

  void SharpSM83::RL(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RL(data));
  }

  uint8_t SharpSM83::RL(uint8_t data) {
    uint32_t wide = (data << 1) | Carry();
    uint8_t carry = static_cast<uint8_t>(wide >> 8);

    ClearNegative();
    ClearHalfCarry();
    SetZero(static_cast<uint8_t>(wide));

    if (carry) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return static_cast<uint8_t>(wide);
  }

  void SharpSM83::RR(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RR(*opPtr);
  }

  void SharpSM83::RR(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RR(data));
  }

  uint8_t SharpSM83::RR(uint8_t data) {
    uint8_t low = data & 1;
    uint8_t res = (data >> 1) | (Carry() << 7);

    ClearNegative();
    ClearHalfCarry();
    SetZero(res);

    if (low) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return res;
  }

  void SharpSM83::SLA(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SLA(*opPtr);
  }

  void SharpSM83::SLA(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SLA(data));
  }

  uint8_t SharpSM83::SLA(uint8_t data) {
    ClearNegative();
    ClearHalfCarry();
    SetZero(data << 1);

    if (data >> 7) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data << 1;
  }

  void SharpSM83::SRA(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SRA(*opPtr);
  }

  void SharpSM83::SRA(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SRA(data));
  }

  uint8_t SharpSM83::SRA(uint8_t data) {
    ClearNegative();
    ClearHalfCarry();
    SetZero(((int8_t) data) >> 1);

    if (data & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return ((int8_t) data) >> 1;
  }

  void SharpSM83::SWAP(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SWAP(*opPtr);
  }

  void SharpSM83::SWAP(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SWAP(data));
  }

  uint8_t SharpSM83::SWAP(uint8_t data) {
    ClearNegative();
    ClearHalfCarry();
    SetZero((data << 4) | (data >> 4));
    ClearCarry();

    return (data << 4) | (data >> 4);
  }

  void SharpSM83::SRL(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SRL(*opPtr);
  }

  void SharpSM83::SRL(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SRL(data));
  }

  uint8_t SharpSM83::SRL(uint8_t data) {
    ClearNegative();
    ClearHalfCarry();
    SetZero(data >> 1);
    if (data & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data >> 1;
  }

  void SharpSM83::BIT(uint8_t bit, SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    BIT(bit, *opPtr);
  }

  void SharpSM83::BIT(uint8_t bit, uint8_t data) {
    auto mask = bitmasks[bit];

    ClearNegative();
    SetHalfCarry();

    if ((data & mask) == 0)
      SetZero();
    else
      ClearZero();

  }

  void SharpSM83::RES(uint8_t bit, SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    auto res = RES(bit, *opPtr);

    *opPtr = res;
  }

  void SharpSM83::RES(uint8_t bit, uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    auto res = RES(bit, data);
    bus->cpuWrite(address, res);
  }

  uint8_t SharpSM83::RES(uint8_t bit, uint8_t data) {
    auto mask = bitmasks[bit];

    return data & ~mask;
  }

  void SharpSM83::SET(uint8_t bit, SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    auto res = SET(bit, *opPtr);

    *opPtr = res;
  }

  void SharpSM83::SET(uint8_t bit, uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    auto res = SET(bit, data);
    bus->cpuWrite(address, res);
  }

  uint8_t SharpSM83::SET(uint8_t bit, uint8_t data) {
    auto mask = bitmasks[bit];

    return data | mask;
  }

  void SharpSM83::JP(uint16_t address) {
    regs.pc = address;
  }

  void SharpSM83::ADD(SharpSM83::Register target, uint8_t data) {
    auto opPtr = RegToPointer(target);

    if (!opPtr) {
      // todo handle this
      return;
    }

    uint32_t res = *opPtr + data;

    ClearZero();
    ClearNegative();
    SetHalfCarry(res);
    SetCarry(res);

    *opPtr = (uint16_t) res;
  }

} // hijo