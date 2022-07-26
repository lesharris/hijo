#include "SharpSM83.h"

namespace hijo {
  SharpSM83::SharpSM83() {
    regs.sp = 0xfffe;
    regs.af = 0x01b0;
    regs.bc = 0x0013;
    regs.de = 0x00d8;
    regs.hl = 0x014d;
    regs.pc = 0;
  }

  void SharpSM83::InitOpcodes() {
    m_Opcodes = {
        {0x0,  "NOP",          1, 4,  {}},
        {0x1,  "LD BC, u16",   3, 12, {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.c = (cpu.latch & 0xFF00) >> 8;
                                            return 0;
                                          },
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.b = cpu.latch & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x2,  "LD (BC), A",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.bus->cpuWrite(cpu.regs.bc, cpu.regs.a);
                                            return 0;
                                          }
                                      }},
        {0x3,  "INC BC",       1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.bc++;
                                            return 0;
                                          }
                                      }},
        {0x4,  "INC B",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.b + 1;

                                            cpu.SetZero(res);
                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.b = res & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0x5,  "DEC B",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.b - 1;

                                            cpu.SetZero(res);
                                            cpu.SetNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.b = res & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0x6,  "LD B, u8",     2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.b = cpu.latch & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0x7,  "RLCA",         1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = (cpu.regs.a << 1) | cpu.Carry() ? 1 : 0;
                                            cpu.ClearZero();
                                            cpu.ClearNegative();
                                            cpu.ClearHalfCarry();
                                            cpu.SetCarry(res);
                                            cpu.regs.a = res & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0x8,  "LD (u16), SP", 3, 20, {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.sp = (cpu.bus->cpuRead(cpu.latch)) << 8;

                                            return 0;
                                          },
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.sp |= cpu.bus->cpuRead(cpu.latch);

                                            return 0;
                                          }
                                      }},
        {0x9,  "ADD HL, BC",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.hl + cpu.regs.bc;

                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);
                                            cpu.SetCarry(res);

                                            cpu.regs.hl = res;

                                            return 0;
                                          },
                                      }},
        {0xA,  "LD A, (BC)",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.a = cpu.bus->cpuRead(cpu.regs.bc);

                                            return 0;
                                          }
                                      }},
        {0xB,  "DEC BC",       1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.bc--;

                                            return 0;
                                          }
                                      }},
        {0xC,  "INC C",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.c + 1;

                                            cpu.SetZero(res);
                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.c = res & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0xD,  "DEC C",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.c - 1;

                                            cpu.SetZero(res);
                                            cpu.SetNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.c = res;

                                            return 0;
                                          }
                                      }},
        {0xE,  "LD C, u8",     2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.c = cpu.latch & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0xF,  "RRCA",         1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint8_t low = cpu.regs.a & 1;
                                            uint16_t res = (cpu.regs.a >> 1) | (low << 7);

                                            cpu.ClearZero();
                                            cpu.ClearHalfCarry();
                                            cpu.ClearNegative();
                                            if (low) {
                                              cpu.SetCarry();
                                            } else {
                                              cpu.ClearCarry();
                                            }

                                            cpu.regs.c = res & 0xFF;

                                            return 0;
                                          }
                                      }},
        {0x10, "STOP",         1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.m_Stopped = true;
                                            return 0;
                                          }
                                      }},
        {0x11, "LD DE, u16",   3, 12, {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.e = (cpu.latch & 0xFF00) >> 8;
                                            return 0;
                                          },
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.d = cpu.latch & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x12, "LD (DE), A",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.bus->cpuWrite(cpu.regs.de, cpu.regs.a);
                                            return 0;
                                          }
                                      }},

        {0x13, "INC DE",       1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.de++;
                                            return 0;
                                          }
                                      }},
        {0x14, "INC D",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.d + 1;

                                            cpu.SetZero(res);
                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.d = res & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x15, "DEC D",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.d - 1;

                                            cpu.SetZero(res);
                                            cpu.SetNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.d = res & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x16, "LD D, u8",     2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.d = cpu.latch & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x16, "RLA",          1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            int wide = (cpu.regs.a << 1) | cpu.Carry() ? 1 : 0;

                                            cpu.ClearNegative();
                                            cpu.ClearHalfCarry();
                                            cpu.ClearZero();
                                            cpu.SetCarry(wide);

                                            return 0;
                                          }
                                      }},
        {0x17, "JR i8",        2, 12, {
                                          [](SharpSM83 &cpu) {
                                            int16_t offset = (int16_t) cpu.regs.pc + (int8_t) (cpu.latch & 0xFF);

                                            cpu.regs.pc = (uint16_t) offset;

                                            return 0;
                                          }
                                      }},
        {0x19, "ADD HL, DE",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.hl + cpu.regs.de;

                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);
                                            cpu.SetCarry(res);

                                            cpu.regs.hl = res;

                                            return 0;
                                          },
                                      }},
        {0x1A, "LD A, (DE)",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.a = cpu.bus->cpuRead(cpu.regs.de);

                                            return 0;
                                          },
                                      }},
        {0x1B, "DEC DE",       1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.de--;

                                            return 0;
                                          },
                                      }},
        {0x1C, "INC E",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.e + 1;

                                            cpu.SetZero(res);
                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);

                                            return 0;
                                          },
                                      }},
        {0x1D, "DEC E",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.e - 1;

                                            cpu.SetZero(res);
                                            cpu.SetNegative();
                                            cpu.SetHalfCarry(res);

                                            return 0;
                                          },
                                      }},
        {0x1E, "LD E, u8",     2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.e = cpu.latch & 0xFF;

                                            return 0;
                                          },
                                      }},
        {0x1F, "RRA",          1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint8_t low = cpu.regs.a & 1;
                                            cpu.regs.a = (cpu.regs.a >> 1) | (cpu.Carry() ? 1 : 0) << 7;

                                            cpu.ClearZero();
                                            cpu.ClearNegative();
                                            cpu.ClearHalfCarry();

                                            if (low) {
                                              cpu.SetCarry();
                                            } else {
                                              cpu.ClearCarry();
                                            }

                                            return 0;
                                          },
                                      }},
        {0x20, "JR NZ, i8",    2, 12, {
                                          [](SharpSM83 &cpu) {

                                            if (cpu.Zero() == 0)
                                              return -4;

                                            int16_t offset = (int16_t) cpu.regs.pc + (int8_t) (cpu.latch & 0xFF);

                                            cpu.regs.pc = (uint16_t) offset;

                                            return 0;
                                          },
                                      }},
        {0x21, "LD HL, u16",   3, 12, {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.l = (cpu.latch & 0xFF00) >> 8;
                                            return 0;
                                          },
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.h = cpu.latch & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x22, "LD (HL+), A",  1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.bus->cpuWrite(cpu.regs.hl++, cpu.regs.a);

                                            return 0;
                                          }
                                      }},
        {0x23, "INC HL",       1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.hl++;

                                            return 0;
                                          }
                                      }},
        {0x24, "INC H",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.h + 1;

                                            cpu.SetZero(res);
                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.h = res & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x25, "DEC H",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.h - 1;

                                            cpu.SetZero(res);
                                            cpu.SetNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.h = res & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x26, "LD H, u8",     2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.h = cpu.latch & 0xFF;
                                            return 0;
                                          }
                                      }},
        {0x27, "DAA",          1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            if (cpu.Negative() != 0) {
                                              if (cpu.HalfCarry() != 0) {
                                                cpu.regs.a += 0xFA;
                                              }
                                              if (cpu.Carry() != 0) {
                                                cpu.regs.a += 0xA0;
                                              }
                                            } else {
                                              uint32_t a = cpu.regs.a;

                                              if ((cpu.regs.a & 0xF) > 0x9 || cpu.HalfCarry() != 0) {
                                                a += 0x6;
                                              }

                                              if ((a & 0x1F0) > 0x90 || cpu.Carry() != 0) {
                                                a += 0x60;
                                                cpu.SetCarry();
                                              } else {
                                                cpu.ClearCarry();
                                              }

                                              cpu.regs.a = (uint8_t) (a & 0xFF);
                                            }

                                            cpu.ClearHalfCarry();
                                            cpu.SetZero(!cpu.regs.a);
                                            return 0;
                                          }
                                      }},
        {0x28, "JR Z, i8",     2, 12, {
                                          [](SharpSM83 &cpu) {

                                            if (cpu.Zero() != 0)
                                              return -4;

                                            int16_t offset = (int16_t) cpu.regs.pc + (int8_t) (cpu.latch & 0xFF);

                                            cpu.regs.pc = (uint16_t) offset;

                                            return 0;
                                          },
                                      }},
        {0x29, "ADD HL, HL",   1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            uint32_t res = cpu.regs.hl + cpu.regs.hl;

                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);
                                            cpu.SetCarry(res);

                                            cpu.regs.hl = (uint16_t) res & 0xFFFF;
                                            return 0;
                                          },
                                      }},
        {0x2A, "LD A, (HL+)",  1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.a = cpu.bus->cpuRead(cpu.regs.hl++);
                                            return 0;
                                          },
                                      }},
        {0x2B, "DEC HL",       1, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.hl--;
                                            return 0;
                                          },
                                      }},
        {0x2C, "INC L",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.l + 1;

                                            cpu.SetZero(res);
                                            cpu.ClearNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.l = res & 0xFF;

                                            return 0;
                                          },
                                      }},
        {0x2D, "DEC L",        1, 4,  {
                                          [](SharpSM83 &cpu) {
                                            uint16_t res = cpu.regs.l - 1;

                                            cpu.SetZero(res);
                                            cpu.SetNegative();
                                            cpu.SetHalfCarry(res);

                                            cpu.regs.l = res & 0xFF;

                                            return 0;
                                          },
                                      }},
        {0x2E, "LD L, u8",     2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.l = cpu.latch & 0xFF;
                                            return 0;
                                          },
                                      }},
        {0x2F, "CPL",          2, 8,  {
                                          [](SharpSM83 &cpu) {
                                            cpu.regs.a ^= 0xFF;
                                            cpu.SetHalfCarry();
                                            cpu.SetNegative();

                                            return 0;
                                          },
                                      }},
    };
  }

  void SharpSM83::ConnectBus(System *system) {
    bus = system;

  }

} // hijo