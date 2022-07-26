#pragma once

#include "System.h"
#include "cpu/SharpSM83.h"

namespace hijo {

  class Gameboy : public System {
  public:
    Gameboy();

  public:
    void cpuWrite(uint16_t addr, uint8_t data) override;

    uint8_t cpuRead(uint16_t addr, bool readOnly = false) override;

  private:
    uint8_t m_Ram[2048]; // temp
    SharpSM83 m_Cpu;
    uint64_t m_CycleCount = 0;
  };

} // hijo

