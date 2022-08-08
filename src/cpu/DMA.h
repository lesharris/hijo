#pragma once

#include <cstdint>

namespace hijo {

  class DMA {
  public:
    void Start(uint8_t start);

    void Tick();

    bool Transferring();

    void Reset();

  private:
    bool active;
    uint8_t byte;
    uint8_t value;
    uint8_t startDelay;
  };

} // hijo

