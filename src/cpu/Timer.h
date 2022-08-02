#pragma once

#include <cstdint>

namespace hijo {

  class Timer {
  public:
    Timer();

    void Tick();

    void Reset();

    void Write(uint16_t address, uint8_t value);

    uint8_t Read(uint16_t address);

  public:
    uint16_t div;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;
  };

} // hijo

