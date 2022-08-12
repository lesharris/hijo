#include "Interrupts.h"
#include "Stack.h"

#include "system/Gameboy.h"
#include <spdlog/spdlog.h>

namespace hijo {
  void Interrupts::RequestInterrupt(SharpSM83 &cpu, const Interrupts::Interrupt &type) {
    uint8_t it = static_cast<uint8_t>(type);
    cpu.m_IF |= it;
  }

  void Interrupts::HandleInterrupts(SharpSM83 &cpu) {
    auto &bus = Gameboy::Get();
    uint8_t interrupt_bit = 0;
    uint8_t queue = cpu.m_IF & cpu.m_IE & 0x1F;

    if (queue) {
      while (!(queue & 1)) {
        queue >>= 1;
        interrupt_bit++;
      }

      cpu.m_Halted = false;
      cpu.m_InterruptMasterEnabled = false;
      cpu.IntFlags(cpu.m_IF & ~(1 << interrupt_bit));

      Stack::Push16(cpu.regs.pc);
      bus.Cycles(4);

      cpu.regs.pc = interrupt_bit * 8 + 0x40;
    }
  }

  void Interrupts::HandleInterrupt(SharpSM83 &cpu, uint16_t addr, uint8_t it) {
    auto &bus = Gameboy::Get();

    uint8_t flags = cpu.IntFlags();
    cpu.IntFlags(flags & ~it);
    cpu.m_Halted = false;
    cpu.m_InterruptMasterEnabled = false;

    bus.Cycles(2);

    Stack::Push16(cpu.regs.pc);
    bus.Cycles(2);

    cpu.regs.pc = addr;
  }

  bool Interrupts::CheckInterrupt(SharpSM83 &cpu, uint16_t addr, const Interrupts::Interrupt &interrupt) {
    uint8_t it = static_cast<uint8_t>(interrupt);

    uint8_t ifs = cpu.IntFlags() & it;
    uint8_t ies = cpu.IERegister() & it;

    if (ifs && ies) {
      HandleInterrupt(cpu, addr, it);
      return true;
    }

    return false;
  }
} // hijo