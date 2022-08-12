#pragma once

#include "SharpSM83.h"

namespace hijo {

  // Interrupt Flags:
  // 0x000J S T L V
  //      | | | | |_ VBlank   0
  //      | | | |___ LCD Stat 1
  //      | | |_____ Timer    2
  //      | |_______ Serial   3
  //      |_________ Joypad   4

  class Interrupts {
  public:
    enum class Interrupt {
      VBlank = 1,
      LCDStat = 2,
      Timer = 4,
      Serial = 8,
      Joypad = 16
    };

  public:
    static void RequestInterrupt(SharpSM83 &cpu, const Interrupt &type);

    static void HandleInterrupts(SharpSM83 &cpu);

  private:
    static void HandleInterrupt(SharpSM83 &cpu, uint16_t addr, uint8_t it);

    static bool CheckInterrupt(SharpSM83 &cpu, uint16_t addr, const Interrupt &interrupt);
  };

} // hijo

