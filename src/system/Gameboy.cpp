#include "Gameboy.h"

namespace hijo {

  Gameboy::Gameboy() {
    m_Cpu.ConnectBus(this);
  }

  void Gameboy::cpuWrite(uint16_t addr, uint8_t data) {
    m_Ram[addr] = data;
  }

  uint8_t Gameboy::cpuRead(uint16_t addr, bool) {
    return m_Ram[addr];
  }

} // hijo