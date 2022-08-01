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
      VBlank = 0,
      LCDStat,
      Timer,
      Serial,
      Joypad
    };

  public:
    static void RequestInterrupt(SharpSM83 &cpu, const Interrupt &type);

    static void HandleInterrupts(SharpSM83 &cpu);

  public:
    static bool VBlank(SharpSM83 &cpu);

    static void VBlank(SharpSM83 &cpu, bool isSet);

    static bool LCDStat(SharpSM83 &cpu);

    static void LCDStat(SharpSM83 &cpu, bool isSet);

    static bool Timer(SharpSM83 &cpu);

    static void Timer(SharpSM83 &cpu, bool isSet);

    static bool Serial(SharpSM83 &cpu);

    static void Serial(SharpSM83 &cpu, bool isSet);

    static bool Joypad(SharpSM83 &cpu);

    static void Joypad(SharpSM83 &cpu, bool isSet);

    static bool GetFlag(SharpSM83 &cpu, const Interrupt &flag);

    static void SetFlag(SharpSM83 &cpu, const Interrupt &flag, bool isSet);

  private:
    static void HandleInterrupt(SharpSM83 &cpu, uint16_t addr);

    static bool CheckInterrupt(SharpSM83 &cpu, uint16_t addr, const Interrupt &interrupt);
  };

} // hijo

