#include "Gameboy.h"

#include "core/Hijo.h"

#include "display/LCD.h"

namespace hijo {

  Gameboy::Gameboy() {
    auto &app = Hijo::Get();

    app.System(this);

    m_Cpu.ConnectBus(this);

    memset(m_ExtRam, 0, 1024 * 8);
    memset(m_WorkRam, 0, 1024 * 8);
    memset(m_HighRam, 0, 127);
    memset(m_Serial, 0, 2);

    /* m_Ram[0] = 0x04; // INC B
     m_Ram[1] = 0x0E; // LD C, #$20
     m_Ram[2] = 0x20;
     m_Ram[3] = 0xCD; // CALL $C00;
     m_Ram[4] = 0x00;
     m_Ram[5] = 0x0C;
     m_Ram[6] = 0xC3; // JP $0000

     m_Ram[0xC00] = 0x14; // INC D
     m_Ram[0xC01] = 0xC9; // RET

     // m_Ram[3] = 0x18; // JR $0000 < -3 >
     // m_Ram[4] = 0xFD;
     //m_Ram[3] = 0xC3; // JP $0000;*/

    EventManager::Get().Attach<
        Events::ExecuteCPU,
        &Gameboy::HandleCPUExecution
    >(this);

    EventManager::Get().Attach<
        Events::StepCPU,
        &Gameboy::HandleCPUStep
    >(this);

    EventManager::Get().Attach<
        Events::ExecuteUntil,
        &Gameboy::HandleExecuteUntil
    >(this);
  }

  Gameboy::~Gameboy() {
    EventManager::Get().DetachAll(this);
  }

  void Gameboy::cpuWrite(uint16_t addr, uint8_t data) {
    if (addr < 0x8000) {
      // Write to rom, configure mapper eventually.
    } else if (addr >= 0x8000 && addr < 0xA000) {
      m_PPU.VRAMWrite(addr, data);
    } else if (addr >= 0xA000 && addr < 0xC000) {
      m_ExtRam[addr & 0x1FFF] = data;
    } else if (addr >= 0xC000 && addr < 0xFE00) {
      m_WorkRam[addr & 0x1FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
      // OAM

      if (m_DMA.Transferring()) {
        return;
      }

      m_PPU.OAMWrite(addr, data);
    } else if (addr >= 0xFEA0 && addr < 0xFF00) {
      // Unusable
    } else if (addr >= 0xFF00 && addr < 0xFF80) {
      switch (addr) {
        case 0xFF00:
          break;

        case 0xFF01:
          m_Serial[0] = data;
          m_Buffer.push_back((char) m_Serial[0]);
          spdlog::get("console")->info("\n{}", m_Buffer);
          break;

        case 0xFF02:
          m_Serial[1] = data;
          break;

        case 0xFF04:
        case 0xFF05:
        case 0xFF06:
        case 0xFF07:
          m_Timer.Write(addr, data);
          break;

        case 0xFF0F:
          m_Cpu.IntFlags(data);
          break;

        default:
          if (addr >= 0xFF10 && addr <= 0xFF3F) {
            // Sound
          }

          if (addr >= 0xFF40 && addr <= 0xFF4B) {
            LCD::Get().Write(addr, data);
          }

          if (addr >= 0xFF80 && addr < 0xFFFF) {
            m_HighRam[addr & 0x7F] = data;
          }
          break;
      }
    } else if (addr == 0xFFFF) {
      m_Cpu.InterruptEnable(data);
    }
  }

  uint8_t Gameboy::cpuRead(uint16_t addr, bool) {
    if (addr < 0x8000) {
      return m_Cartridge->Data()[addr];
    } else if (addr >= 0x8000 && addr < 0xA000) {
      return m_PPU.VRAMRead(addr);
    } else if (addr >= 0xA000 && addr < 0xC000) {
      return m_ExtRam[addr & 0x1FFF];
    } else if (addr >= 0xC000 && addr < 0xFE00) {
      return m_WorkRam[addr & 0x1FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
      // OAM
      if (m_DMA.Transferring()) {
        return 0xFF;
      }

      return m_PPU.OAMRead(addr);
    } else if (addr >= 0xFEA0 && addr < 0xFF00) {
      // Unusable
      return 0;
    } else if (addr >= 0xFF00 && addr < 0xFF80) {
      switch (addr) {
        case 0xFF00:  // Gamepad
          return 0xFF;

        case 0xFF01:
          return m_Serial[0];

        case 0xFF02:
          return m_Serial[1];

        case 0xFF04:
        case 0xFF05:
        case 0xFF06:
        case 0xFF07:
          return m_Timer.Read(addr);

        case 0xFF0F:
          return m_Cpu.IntFlags();

        default:
          if (addr >= 0xFF10 && addr <= 0xFF3F) {
            // Sound
            return 0;
          }

          if (addr >= 0xFF40 && addr <= 0xFF4B) {
            return LCD::Get().Read(addr);
          }
      }
      // Notify?
      return 0;
    } else if (addr >= 0xFF80 && addr < 0xFFFF) {
      return m_HighRam[addr & 0x7F];
    } else if (addr == 0xFFFF) {
      return m_Cpu.InterruptEnable();
    }
  }

  void Gameboy::Update(double) {
    // 154 Scanlines per Frame
    // 456 tcycles per scanline
    // 70224 tcycles per frame

    if (m_Run) {
      for (auto cycle = 0; cycle < 70224; cycle++) {

        if (m_TargetActive && m_Cpu.GetRegisters().pc == m_TargetAddr) {
          m_TargetActive = false;
          m_Run = false;
          return;
        }

        m_Cpu.Cycle(1);
        m_CycleCount += 1;

        for (auto n = 0; n < 4; n++) {
          m_Timer.Tick();
          m_PPU.Tick();
        }

        m_DMA.Tick();
      }
    }
  }

  void Gameboy::HandleCPUExecution(const Events::ExecuteCPU &event) {
    m_Run = event.execute;
  }

  void Gameboy::HandleCPUStep(const Events::StepCPU &) {
    auto cycles = m_Cpu.Step();

    for (auto n = 0; n < 4; n++)
      m_PPU.Tick();

    m_CycleCount += cycles;
  }

  void Gameboy::InsertCartridge(const std::string &path) {
    m_Cartridge = Cartridge::Load(path);
  }

  void Gameboy::HandleExecuteUntil(const Events::ExecuteUntil &event) {
    m_TargetAddr = event.addr;
    m_Run = true;
    m_TargetActive = true;
  }

} // hijo