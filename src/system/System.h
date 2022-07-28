#pragma once

#include <cstdint>

namespace hijo {
  class System {
  public:
    virtual void cpuWrite(uint16_t addr, uint8_t data) = 0;

    virtual uint8_t cpuRead(uint16_t addr, bool readOnly = false) = 0;

    virtual void Update(double timestep) = 0;
  };
}
