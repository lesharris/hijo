#include "Interrupts.h"
#include "Stack.h"

#include "system/Gameboy.h"

namespace hijo {
  void Interrupts::RequestInterrupt(SharpSM83 &cpu, const Interrupts::Interrupt &type) {
    uint8_t it = static_cast<uint8_t>(type);
    uint8_t flags = cpu.IntFlags();
    cpu.IntFlags(flags | it);
  }

  void Interrupts::HandleInterrupts(SharpSM83 &cpu) {
    if (CheckInterrupt(cpu, 0x40, Interrupt::VBlank)) {}
    else if (CheckInterrupt(cpu, 0x48, Interrupt::LCDStat)) {}
    else if (CheckInterrupt(cpu, 0x50, Interrupt::Timer)) {}
    else if (CheckInterrupt(cpu, 0x58, Interrupt::Serial)) {}
    else if (CheckInterrupt(cpu, 0x60, Interrupt::Joypad)) {}
  }

  void Interrupts::HandleInterrupt(SharpSM83 &cpu, uint16_t addr) {
    auto &bus = Gameboy::Get();

    bus.Cycles(2);

    Stack::Push16(cpu.regs.pc);
    bus.Cycles(2);

    cpu.regs.pc = addr;
    bus.Cycles(1);
  }

  bool Interrupts::CheckInterrupt(SharpSM83 &cpu, uint16_t addr, const Interrupts::Interrupt &interrupt) {
    uint8_t it = static_cast<uint8_t>(interrupt);

    if (cpu.IntFlags() & it && cpu.IERegister() & it) {
      HandleInterrupt(cpu, addr);
      uint8_t flags = cpu.IntFlags();
      cpu.IntFlags(flags & ~it);
      cpu.m_Halted = false;
      cpu.m_InterruptMasterEnabled = false;

      return true;
    }

    return false;
  }
} // hijo