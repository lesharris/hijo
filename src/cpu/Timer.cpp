#include "Timer.h"
#include "system/Gameboy.h"
#include "Interrupts.h"
#include <spdlog/spdlog.h>

namespace hijo {
  Timer::Timer() {
    Reset();
  }

  void Timer::Tick() {
    uint16_t prev = div;
    div++;

    bool update = false;

    switch (tac & 0x3) {
      case 0:
        update = (prev & (1 << 9)) && (!(div & (1 << 9)));
        break;

      case 1:
        update = (prev & (1 << 3)) && (!(div & (1 << 3)));
        break;

      case 2:
        update = (prev & (1 << 5)) && (!(div & (1 << 5)));
        break;

      case 3:
        update = (prev & (1 << 7)) && (!(div & (1 << 7)));
        break;
    }

    if (update && tac & (1 << 2)) {
      tima++;

      if (tima == 0xFF) {
        tima = tma;

        auto &bus = Gameboy::Get();
        Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::Timer);
      }
    }
  }

  void Timer::Reset() {
    div = 0xAC00;
  }

  void Timer::Write(uint16_t address, uint8_t value) {
    switch (address) {
      case 0xFF04:
        div = 0;
        break;

      case 0xFF05:
        tima = value;
        break;

      case 0xFF06:
        tma = value;
        break;

      case 0xFF07:
        tac = value;
        break;
    }
  }

  uint8_t Timer::Read(uint16_t address) {
    switch (address) {
      case 0xFF04:
        return div >> 8;

      case 0xFF05:
        return tima;

      case 0xFF06:
        return tma;

      case 0xFF07:
        return tac;

      default: {
        spdlog::get("console")->warn("Invalid Timer Address passed: {:04X}", address);
        return 0;
      }
    }
  }
} // hijo