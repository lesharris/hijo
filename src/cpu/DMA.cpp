#include "DMA.h"

#include "system/Gameboy.h"

namespace hijo {
  void DMA::Start(uint8_t start) {
    active = true;
    byte = 0;
    startDelay = 2;
    value = start;
  }

  void DMA::Tick() {
    if (!active) {
      return;
    }

    if (startDelay) {
      startDelay--;
      return;
    }

    auto &bus = Gameboy::Get();
    auto &ppu = bus.m_PPU;

    uint16_t addr = (value * 0x100) + byte;

    ppu.OAMWrite(0xFE00 | byte, bus.cpuRead(addr));

    byte++;
    active = byte < 0xA0;
  }

  bool DMA::Transferring() {
    return active;
  }

  void DMA::Reset() {
    active = false;
  }
} // hijo