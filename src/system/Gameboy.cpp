#include "Gameboy.h"
#include <spdlog/spdlog.h>

namespace hijo {

  Gameboy::Gameboy() {
    m_Cpu.ConnectBus(this);

    m_Ram[0] = 0x3C; // INC A
    m_Ram[1] = 0xC3; // JP $0000;
    m_Ram[2] = 0;
    m_Ram[3] = 0;
  }

  void Gameboy::cpuWrite(uint16_t addr, uint8_t data) {
    m_Ram[addr] = data;
  }

  uint8_t Gameboy::cpuRead(uint16_t addr, bool) {
    return m_Ram[addr];
  }

  void Gameboy::Update(double) {
    m_Cpu.Cycle(32);
    const auto &regs = m_Cpu.GetRegisters();
    spdlog::get("console")->info("{}", regs.a);
  }

} // hijo