#pragma once

#include <cstring>

#include "System.h"
#include "core/events/EventManager.h"
#include "cpu/SharpSM83.h"

namespace hijo {

  class Gameboy : public System {
  public:
    Gameboy();

    ~Gameboy();

  public:
    void cpuWrite(uint16_t addr, uint8_t data) override;

    uint8_t cpuRead(uint16_t addr, bool readOnly = false) override;

    void Update(double timestep) override;

    bool Running() const {
      return m_Run;
    }
    
  private:
    void HandleCPUExecution(const Events::ExecuteCPU &event);

    void HandleCPUStep(const Events::StepCPU &event);

  private:
    friend class UI;

  private:
    uint8_t m_Ram[65536]; // temp
    SharpSM83 m_Cpu;
    bool m_Run = false;
    // uint64_t m_CycleCount = 0;
  };

} // hijo

