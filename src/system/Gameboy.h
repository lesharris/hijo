#pragma once

#include <cstring>
#include <vector>

#include "System.h"
#include "core/events/EventManager.h"

#include "cpu/SharpSM83.h"
#include "cpu/Timer.h"
#include "cpu/DMA.h"
#include "cartridge/Cartridge.h"
#include "display/PPU.h"

namespace hijo {

  class Gameboy : public System {
  public:
    static Gameboy &Get() {
      static Gameboy instance;

      return instance;
    }

  private:
    Gameboy();

    ~Gameboy();

  public:
    void cpuWrite(uint16_t addr, uint8_t data) override;

    void cpuWrite16(uint16_t addr, uint16_t value);

    uint8_t cpuRead(uint16_t addr, bool readOnly = false) override;

    uint16_t cpuRead16(uint16_t addr);

    void Cycles(uint32_t cycles);

  public:
    void Update(double timestep) override;

    void InsertCartridge(const std::string &path);

    bool Running() const {
      return m_Run;
    }

    uint64_t Cycles() const {
      return m_CycleCount;
    }

    std::vector<Color> &VideoBuffer() {
      return m_PPU.VideoBuffer();
    }

  private:
    void HandleCPUExecution(const Events::ExecuteCPU &event);

    void HandleCPUStep(const Events::StepCPU &event);

    void HandleExecuteUntil(const Events::ExecuteUntil &event);

  private:
    friend class UI;

    friend class PPU;

    friend class Timer;

    friend class DMA;

    friend class LCD;

    friend class Stack;

    friend class Hijo;

    friend class SharpSM83;

  private:
    bool m_Run = false;
    uint64_t m_CycleCount = 0;

    uint8_t m_ExtRam[1024 * 8];
    uint8_t m_WorkRam[1024 * 8];
    uint8_t m_HighRam[127];
    uint8_t m_Serial[2];

    SharpSM83 m_Cpu;
    Timer m_Timer;
    PPU m_PPU;
    DMA m_DMA;
    std::shared_ptr<Cartridge> m_Cartridge;

    uint16_t m_TargetAddr = 0;
    bool m_TargetActive = false;

    std::string m_Buffer;
  };

} // hijo

