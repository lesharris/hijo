#include "Gameboy.h"

#include "core/Hijo.h"

#include "display/LCD.h"

namespace hijo {

  Gameboy::Gameboy() {
    auto &app = Hijo::Get();

    app.System(this);

    memset(m_ExtRam, 0, 1024 * 8);
    memset(m_WorkRam, 0, 1024 * 8);
    memset(m_HighRam, 0, 127);
    memset(m_Serial, 0, 2);

    m_Timer.div = 0xABCC;

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
      m_Cartridge->Write(addr, data);
    } else if (addr < 0xA000) {
      //Char/Map Data
      m_PPU.VRAMWrite(addr, data);
    } else if (addr < 0xC000) {
      m_Cartridge->Write(addr, data);
    } else if (addr < 0xE000) {
      //WRAM
      m_WorkRam[addr & 0x1FFF] = data;
    } else if (addr < 0xFE00) {
      m_WorkRam[addr & 0x1FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
      //OAM
      if (m_DMA.Transferring()) {
        return;
      }

      m_PPU.OAMWrite(addr, data);
    } else if (addr < 0xFF00) {
      //unusable reserved
    } else if (addr < 0xFF80) {
      //IO Registers...
      if (addr == 0xFF00) {
        m_Controller.SetSelected(data);
        return;
      }

      if (addr == 0xFF01) {
        m_Serial[0] = data;
        if (m_SerialTransfer)
          m_Buffer.push_back((char) m_Serial[0]);
        return;
      }

      if (addr == 0xFF02) {
        m_Serial[1] = data;
        if (!m_SerialTransfer && BIT(m_Serial[1], 7)) {
          m_SerialTransfer = true;
          m_Buffer.clear();
        } else if (m_SerialTransfer && !(BIT(m_Serial[1], 7))) {
          spdlog::get("console")->info("\n{}", m_Buffer);
          m_SerialTransfer = false;
        }
        return;
      }

      if (BETWEEN(addr, 0xFF04, 0xFF07)) {
        m_Timer.Write(addr, data);
        return;
      }

      if (addr == 0xFF0F) {
        m_Cpu.IntFlags(data);
        return;
      }

      if (BETWEEN(addr, 0xFF10, 0xFF3F)) {
        //ignore sound
        return;
      }

      if (BETWEEN(addr, 0xFF40, 0xFF4B)) {
        LCD::Get().Write(addr, data);
        return;
      }

      if (addr == 0xFF4F) {
        // CGB Vram Bank Select
        return;
      }

      if (addr == 0xFF50) {
        // Boot rom
        return;
      }

      if (BETWEEN(addr, 0xFF51, 0xFF55)) {
        // CGB Vram DATA
        return;
      }

      if (BETWEEN(addr, 0xFF68, 0xFF69)) {
        // CGB BG/Obj Pals
        return;
      }

      if (BETWEEN(addr, 0xFF70, 0xFF7F)) {
        // CGB WRAM Bank Select
        return;
      }

      spdlog::get("console")->info("UNSUPPORTED bus_write({:04X})", addr);
    } else if (addr == 0xFFFF) {
      //CPU SET ENABLE REGISTER

      m_Cpu.IERegister(data);
    } else {
      m_HighRam[addr & 0x7F] = data;
    }
  }

  uint8_t Gameboy::cpuRead(uint16_t addr) {
    if (addr < 0x8000) {
      //ROM Data
      return m_Cartridge->Read(addr);
    } else if (addr < 0xA000) {
      //Char/Map Data
      return m_PPU.VRAMRead(addr);
    } else if (addr < 0xC000) {
      //Cartridge RAM
      return m_Cartridge->Read(addr);
    } else if (addr < 0xE000) {
      //WRAM (Working RAM)
      return m_WorkRam[addr & 0x1FFF];
    } else if (addr < 0xFE00) {
      return m_WorkRam[addr & 0x1FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
      //OAM
      if (m_DMA.Transferring()) {
        return 0xFF;
      }

      return m_PPU.OAMRead(addr);
    } else if (addr < 0xFF00) {
      //reserved unusable...
      return 0;
    } else if (addr < 0xFF80) {
      //IO Registers...
      if (addr == 0xFF00) {
        return m_Controller.Output();
      }

      if (addr == 0xFF01) {
        return m_Serial[0];
      }

      if (addr == 0xFF02) {
        return m_Serial[1];
      }

      if (BETWEEN(addr, 0xFF04, 0xFF07)) {
        return m_Timer.Read(addr);
      }

      if (addr == 0xFF0F) {
        return m_Cpu.IntFlags();
      }

      if (BETWEEN(addr, 0xFF10, 0xFF3F)) {
        //ignore sound
        return 0;
      }

      if (BETWEEN(addr, 0xFF40, 0xFF4B)) {
        return LCD::Get().Read(addr);
      }

      if (addr == 0xFF4F) {
        // CGB Vram Bank Select
        return 0;
      }

      if (addr == 0xFF50) {
        // Boot rom
        return 0;
      }

      if (BETWEEN(addr, 0xFF51, 0xFF55)) {
        // CGB Vram DATA
        return 0;
      }

      if (BETWEEN(addr, 0xFF68, 0xFF69)) {
        // CGB BG/Obj Pals
        return 0;
      }

      if (BETWEEN(addr, 0xFF70, 0xFF7F)) {
        // CGB WRAM Bank Select
        return 0;
      }

      spdlog::get("console")->info("UNSUPPORTED bus_read({:04X})\n", addr);
      return 0;
    } else if (addr == 0xFFFF) {
      //CPU ENABLE REGISTER...
      return m_Cpu.IERegister();
    }

    //NO_IMPL
    return m_HighRam[addr & 0x7F];
  }

  void Gameboy::Update(double) {
    // 154 Scanlines per Frame
    // 456 tcycles per scanline
    // 70224 tcycles per frame
    // 17556 mcycles per frame

    m_CyclesTaken = 0;

    if (m_Run) {
      do {
        if (m_TargetActive && m_Cpu.regs.pc == m_TargetAddr) {
          m_TargetActive = false;
          m_Run = false;
          return;
        }

        m_Cpu.Step();
      } while (m_CyclesTaken <= 17556);
    }
  }

  void Gameboy::HandleCPUExecution(const Events::ExecuteCPU &event) {
    m_Run = event.execute;
  }

  void Gameboy::HandleCPUStep(const Events::StepCPU &) {
    m_Cpu.Step();
  }

  void Gameboy::InsertCartridge(const std::string &path) {
    m_Cartridge = Cartridge::Load(path);
  }

  void Gameboy::HandleExecuteUntil(const Events::ExecuteUntil &event) {
    m_TargetAddr = event.addr;
    m_Run = true;
    m_TargetActive = true;
  }

  void Gameboy::Cycles(uint32_t cycles) {
    m_CyclesTaken += cycles;

    for (uint32_t i = 0; i < cycles; i++) {
      for (auto n = 0; n < 4; n++) {
        m_CycleCount++;
        m_Timer.Tick();
        m_PPU.Tick();
      }

      m_DMA.Tick();
    }
  }

  void Gameboy::cpuWrite16(uint16_t addr, uint16_t value) {
    cpuWrite(addr + 1, (value >> 8) & 0xFF);
    cpuWrite(addr, value & 0xFF);
  }

  uint16_t Gameboy::cpuRead16(uint16_t addr) {
    uint16_t low = cpuRead(addr);
    uint16_t high = cpuRead(addr + 1);

    return low | (high << 8);
  }

} // hijo