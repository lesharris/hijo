#pragma once

#include <cstring>
#include <vector>
#include <deque>

#include "System.h"
#include "core/events/EventManager.h"

#include "cpu/SharpSM83.h"
#include "cpu/Timer.h"
#include "cpu/DMA.h"
#include "cartridge/Cartridge.h"
#include "display/PPU.h"
#include "input/Controller.h"
#include "sound/audio/Gb_Apu.h"
#include "sound/audio/Multi_Buffer.h"
#include "sound/Sound_Queue.h"

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

    uint8_t cpuRead(uint16_t addr) override;

    uint16_t cpuRead16(uint16_t addr);

    void Cycles(uint32_t cycles);

  public:
    void Update(double timestep) override;

    void InsertCartridge(const std::string &path);

    bool Running() const {
      return m_Run;
    }

    uint64_t Cycles() const {
      return m_TCycleCount;
    }

    std::vector<Color> &VideoBuffer() {
      return m_PPU.VideoBuffer();
    }

    bool CartridgeLoaded() {
      return m_Cartridge != nullptr;
    }

    void Reset(bool clearCartridge = true);

    /* Events */
  private:
    void HandleCPUExecution(const Events::ExecuteCPU &event);

    void HandleCPUStep(const Events::StepCPU &event);

    void HandleExecuteUntil(const Events::ExecuteUntil &event);

    void HandleLoadRom(const Events::LoadROM &event);

    void HandleReset(const Events::Reset &event);

    void HandleUnloadRom(const Events::UnloadROM &event);

  private:
    friend class UI;

    friend class PPU;

    friend class Timer;

    friend class DMA;

    friend class LCD;

    friend class Stack;

    friend class Hijo;

    friend class SharpSM83;

    friend class Controller;

    friend class Display;

  private:
    bool m_Run = false;

    uint32_t m_TCycleCount = 0;
    uint32_t m_MCycleCount = 0;

    // RAM
    uint8_t m_WorkRam[1024 * 8];
    uint8_t m_HighRam[127];

    // Serial
    uint8_t m_Serial[2];
    bool m_SerialTransfer = false;
    bool m_ControlSet = false;
    uint8_t m_ControlCount = 0;
    std::string m_Buffer;

    // Things on the bus
    SharpSM83 m_Cpu;
    Timer m_Timer;
    PPU m_PPU;
    DMA m_DMA;
    std::shared_ptr<Cartridge> m_Cartridge;
    Controller m_Controller;

    // Manual Stepping State
    uint16_t m_TargetAddr = 0;
    bool m_TargetActive = false;

    // APU
    Sound_Queue m_SoundQueue;
    Gb_Apu m_APU{};
    Stereo_Buffer m_StereoBuffer{};
    blip_sample_t m_SampleBuffer[4096];
    Sound_Queue::sample_t m_AudioBuffer[4096];
    uint32_t m_SampleCount = 0;
  };

} // hijo

