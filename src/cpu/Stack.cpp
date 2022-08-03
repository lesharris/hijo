#include "system/Gameboy.h"

#include "Stack.h"

namespace hijo {
  void Stack::Push(uint8_t data) {
    auto &bus = Gameboy::Get();
    auto &regs = bus.m_Cpu.regs;

    regs.sp--;
    bus.cpuWrite(regs.sp, data);
  }

  void Stack::Push16(uint16_t data) {
    Push((data >> 8) & 0xFF);
    Push(data & 0xFF);
  }

  uint8_t Stack::Pop() {
    auto &bus = Gameboy::Get();
    auto &regs = bus.m_Cpu.regs;

    return bus.cpuRead(regs.sp++);
  }

  uint16_t Stack::Pop16() {
    uint16_t low = Pop();
    uint16_t high = Pop();

    return (high << 8) | low;
  }
} // hijo