#include "Interrupts.h"

namespace hijo {
  bool Interrupts::VBlank(SharpSM83 &cpu) {
    return GetFlag(cpu, Interrupt::VBlank);
  }

  void Interrupts::VBlank(SharpSM83 &cpu, bool isSet) {
    SetFlag(cpu, Interrupt::VBlank, isSet);
  }

  bool Interrupts::LCDStat(SharpSM83 &cpu) {
    return GetFlag(cpu, Interrupt::LCDStat);
  }

  void Interrupts::LCDStat(SharpSM83 &cpu, bool isSet) {
    SetFlag(cpu, Interrupt::LCDStat, isSet);
  }

  bool Interrupts::Timer(SharpSM83 &cpu) {
    return GetFlag(cpu, Interrupt::Timer);
  }

  void Interrupts::Timer(SharpSM83 &cpu, bool isSet) {
    SetFlag(cpu, Interrupt::Timer, isSet);
  }

  bool Interrupts::Serial(SharpSM83 &cpu) {
    return GetFlag(cpu, Interrupt::Serial);
  }

  void Interrupts::Serial(SharpSM83 &cpu, bool isSet) {
    SetFlag(cpu, Interrupt::Serial, isSet);
  }

  bool Interrupts::Joypad(SharpSM83 &cpu) {
    return GetFlag(cpu, Interrupt::Joypad);
  }

  void Interrupts::Joypad(SharpSM83 &cpu, bool isSet) {
    SetFlag(cpu, Interrupt::Joypad, isSet);
  }

  bool Interrupts::GetFlag(SharpSM83 &cpu, const Interrupts::Interrupt &flag) {
    size_t index = static_cast<size_t>(flag);
    return ((cpu.regs.ie & cpu.bitmasks[index]) >> index) == 1;
  }

  void Interrupts::SetFlag(SharpSM83 &cpu, const Interrupts::Interrupt &flag, bool isSet) {
    size_t index = static_cast<size_t>(flag);

    if (isSet)
      cpu.regs.ie |= cpu.bitmasks[index];
    else
      cpu.regs.ie &= ~cpu.bitmasks[index];
  }

  void Interrupts::RequestInterrupt(SharpSM83 &cpu, const Interrupts::Interrupt &type) {
    SetFlag(cpu, type, true);
  }

  void Interrupts::HandleInterrupts(SharpSM83 &cpu) {
    if (CheckInterrupt(cpu, 0x40, Interrupt::VBlank)) {}
    else if (CheckInterrupt(cpu, 0x48, Interrupt::LCDStat)) {}
    else if (CheckInterrupt(cpu, 0x50, Interrupt::Timer)) {}
    else if (CheckInterrupt(cpu, 0x58, Interrupt::Serial)) {}
    else if (CheckInterrupt(cpu, 0x60, Interrupt::Joypad)) {}
  }

  void Interrupts::HandleInterrupt(SharpSM83 &cpu, uint16_t addr) {
    uint8_t high = (cpu.regs.pc & 0xFF00) >> 8;
    uint8_t low = cpu.regs.pc & 0xFF;

    cpu.bus->cpuWrite(--cpu.regs.sp, high);
    cpu.bus->cpuWrite(--cpu.regs.sp, low);

    cpu.regs.pc = addr;
  }

  bool Interrupts::CheckInterrupt(SharpSM83 &cpu, uint16_t addr, const Interrupts::Interrupt &interrupt) {
    if (GetFlag(cpu, interrupt)) {
      HandleInterrupt(cpu, addr);
      SetFlag(cpu, interrupt, false);
      cpu.m_Halted = false;
      cpu.m_Stopped = false;
      cpu.m_InterruptsEnabled = false;

      return true;
    }

    return false;
  }
} // hijo