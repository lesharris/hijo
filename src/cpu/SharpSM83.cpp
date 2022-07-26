#include "SharpSM83.h"

namespace hijo {
  SharpSM83::SharpSM83() {
    Reset();
  }

  void SharpSM83::InitOpcodes() {
    m_Opcodes = {
        {0x0,  "NOP",          AddressingMode::Implied,           1, 4,
            []() {
              return 0;
            }
        },
        {0x1,  "LD BC, u16",   AddressingMode::ExtendedImmediate, 3, 12,
            [this]() {
              LD(Register::BC, latch);
              return 0;
            }
        },
        {0x2,  "LD (BC), A",   AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(latch, regs.a);
              return 0;
            }},
        {0x3,  "INC BC",       AddressingMode::Implied,           1, 8,
            [this]() {
              INC(Register::BC);
              return 0;
            }
        },
        {0x4,  "INC B",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::B);
              return 0;
            }
        },
        {0x5,  "DEC B",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::B);
              return 0;
            }
        },
        {0x6,  "LD B, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::B,
                 static_cast<uint8_t>(latch & 0xFF));

              return 0;
            }
        },
        {0x7,  "RLCA",         AddressingMode::Implied,           1, 4,
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
        {0x8,  "LD (u16), SP", AddressingMode::Extended,          3, 20,
            [this]() {
              LD(latch, regs.sp);

              return 0;
            }
        },
        {0x9,  "ADD HL, BC",   AddressingMode::Implied,           1, 8,
            [this]() {
              ADD(Register::HL, Register::BC, true);
              return 0;
            },
        },
        {0xA,  "LD A, (BC)",   AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(latch));
              return 0;
            }
        },
        {0xB,  "DEC BC",       AddressingMode::Implied,           1, 8,
            [this]() {
              DEC(Register::BC);
              return 0;
            }
        },
        {0xC,  "INC C",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::C);

              return 0;
            }
        },
        {0xD,  "DEC C",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::C);

              return 0;
            }
        },
        {0xE,  "LD C, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::C,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0xF,  "RRCA",         AddressingMode::Implied,           1, 4,
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
        {0x10, "STOP",         AddressingMode::Implied,           1, 4,
            [this]() {
              m_Stopped = true;
              return 0;
            }
        },
        {0x11, "LD DE, u16",   AddressingMode::ExtendedImmediate, 3, 12,
            [this]() {
              LD(Register::DE, latch);
              return 0;
            }},
        {0x12, "LD (DE), A",   AddressingMode::RegisterIndirect,  1, 8,
            [this]() {

              LD(regs.de, regs.a);
              return 0;
            }
        },
        {0x13, "INC DE",       AddressingMode::Implied,           1, 8,
            [this]() {
              DEC(Register::DE);

              return 0;
            }
        },
        {0x14, "INC D",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::D);
              return 0;
            }
        },
        {0x15, "DEC D",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::D);

              return 0;
            }
        },
        {0x16, "LD D, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::D,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0x17, "RLA",          AddressingMode::Implied,           1, 4,
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
        {0x18, "JR i8",        AddressingMode::Relative,          2, 12,
            [this]() {
              int16_t offset = (int16_t) regs.pc +
                               (int8_t) (latch &
                                         0xFF);

              regs.pc = (uint16_t) offset;

              return 0;
            }
        },
        {0x19, "ADD HL, DE",   AddressingMode::Implied,           1, 8,
            [this]() {
              ADD(Register::HL, Register::DE, true);

              return 0;
            },
        },
        {0x1A, "LD A, (DE)",   AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.de));

              return 0;
            },
        },
        {0x1B, "DEC DE",       AddressingMode::Implied,           1, 8,
            [this]() {
              DEC(Register::DE);

              return 0;
            },
        },
        {0x1C, "INC E",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::E);

              return 0;
            },
        },
        {0x1D, "DEC E",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::E);

              return 0;
            },
        },
        {0x1E, "LD E, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::E,
                 static_cast<uint8_t>(latch & 0xFF));

              return 0;
            },
        },
        {0x1F, "RRA",          AddressingMode::Implied,           1, 4,
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
        {0x20, "JR NZ, i8",    AddressingMode::Relative,          2, 12,
            [this]() {

              if (Zero() == 0)
                return -4;

              int16_t offset = (int16_t) regs.pc +
                               (int8_t) (latch &
                                         0xFF);

              regs.pc = (uint16_t) offset;

              return 0;
            },
        },
        {0x21, "LD HL, u16",   AddressingMode::ExtendedImmediate, 3, 12,
            [this]() {
              LD(Register::HL, latch);
              return 0;
            }
        },
        {0x22, "LD (HL+), A",  AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(regs.hl++, regs.a);

              return 0;
            }
        },
        {0x23, "INC HL",       AddressingMode::Implied,           1, 8,
            [this]() {
              INC(Register::HL);

              return 0;
            }
        },
        {0x24, "INC H",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::H);

              return 0;
            }
        },
        {0x25, "DEC H",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::H);

              return 0;
            }
        },
        {0x26, "LD H, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::H,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            }
        },
        {0x27, "DAA",          AddressingMode::Implied,           1, 4,
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
        {0x28, "JR Z, i8",     AddressingMode::Relative,          2, 12,
            [this]() {

              if (Zero() != 0)
                return -4;

              int16_t offset = (int16_t) regs.pc +
                               (int8_t) (latch &
                                         0xFF);

              regs.pc = (uint16_t) offset;

              return 0;
            },
        },
        {0x29, "ADD HL, HL",   AddressingMode::Implied,           1, 8,
            [this]() {
              ADD(Register::HL, Register::HL);
              return 0;
            },
        },
        {0x2A, "LD A, (HL+)",  AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.hl++));
              return 0;
            },
        },
        {0x2B, "DEC HL",       AddressingMode::Implied,           1, 8,
            [this]() {
              DEC(Register::HL);
              return 0;
            },
        },
        {0x2C, "INC L",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::L);
              return 0;
            },
        },
        {0x2D, "DEC L",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::L);
              return 0;
            },
        },
        {0x2E, "LD L, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::L,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            },
        },

        {0x2F, "CPL",          AddressingMode::Implied,           2, 8,
            [this]() {
              regs.a ^= 0xFF;
              SetHalfCarry();
              SetNegative();
              return 0;
            },
        },
        {0x30, "JR NC, i8",    AddressingMode::Relative,          2, 12,
            [this]() {

              if (Carry())
                return -4;

              int16_t offset = (int16_t) regs.pc +
                               (int8_t) (latch &
                                         0xFF);

              regs.pc = (uint16_t) offset;

              return 0;
            },
        },
        {0x31, "LD SP, u16",   AddressingMode::ExtendedImmediate, 3, 12,
            [this]() {
              LD(Register::SP, latch);
              return 0;
            }
        },
        {0x32, "LD (HL-), A",  AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(regs.hl--, regs.a);
              return 0;
            }
        },
        {0x33, "INC SP",       AddressingMode::Implied,           1, 8,
            [this]() {
              INC(Register::SP);
              return 0;
            }
        },
        {0x34, "INC (HL)",     AddressingMode::RegisterIndirect,  1, 12,
            [this]() {
              INC(regs.hl);
              return 0;
            }
        },
        {0x35, "DEC (HL)",     AddressingMode::RegisterIndirect,  1, 12,
            [this]() {
              DEC(regs.hl);
              return 0;
            }
        },
        {0x36, "LD (HL), u8",  AddressingMode::RegisterIndirect,  2, 12,
            [this]() {
              LD(regs.hl,
                 static_cast<uint8_t>(latch & 0xFF));

              return 0;
            }
        },
        {0x37, "SCF",          AddressingMode::Implied,           1, 4,
            [this]() {
              ClearNegative();
              ClearHalfCarry();
              SetCarry();
              return 0;
            }
        },
        {0x38, "JR C, i8",     AddressingMode::Relative,          2, 12,
            [this]() {

              if (Carry() == 0)
                return -4;

              int16_t offset = (int16_t) regs.pc +
                               (int8_t) (latch &
                                         0xFF);

              regs.pc = (uint16_t) offset;

              return 0;
            },
        },
        {0x39, "ADD HL, SP",   AddressingMode::Implied,           1, 8,
            [this]() {
              ADD(Register::HL, Register::SP, true);
              return 0;
            },
        },
        {0x3A, "LD A, (HL-)",  AddressingMode::RegisterIndirect,  1, 8,
            [this]() {
              LD(Register::A, bus->cpuRead(regs.hl--));
              return 0;
            },
        },
        {0x3B, "DEC SP",       AddressingMode::Implied,           1, 8,
            [this]() {
              DEC(Register::SP);
              return 0;
            },
        },
        {0x3C, "INC A",        AddressingMode::Register,          1, 4,
            [this]() {
              INC(Register::A);
              return 0;
            },
        },
        {0x3D, "DEC A",        AddressingMode::Register,          1, 4,
            [this]() {
              DEC(Register::A);
              return 0;
            },
        },
        {0x3E, "LD A, u8",     AddressingMode::Immediate,         2, 8,
            [this]() {
              LD(Register::A,
                 static_cast<uint8_t>(latch & 0xFF));
              return 0;
            },
        },
        {0x3F, "CCF",          AddressingMode::Implied,           1, 4,
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
        }
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

      if (!targetPtr || !opPtr) {
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

} // hijo