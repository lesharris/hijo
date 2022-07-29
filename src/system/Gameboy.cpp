#include "Gameboy.h"

#include "core/Hijo.h"

namespace hijo {

  Gameboy::Gameboy() {
    auto &app = Hijo::Get();

    app.System(this);

    m_Cpu.ConnectBus(this);

    memset(m_Ram, 0, 64 * 1024);

    m_Ram[0] = 0x04; // INC B
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
    //m_Ram[3] = 0xC3; // JP $0000;

    EventManager::Get().Attach<
        Events::ExecuteCPU,
        &Gameboy::HandleCPUExecution
    >(this);

    EventManager::Get().Attach<
        Events::StepCPU,
        &Gameboy::HandleCPUStep
    >(this);
  }

  Gameboy::~Gameboy() {
    EventManager::Get().DetachAll(this);
  }

  void Gameboy::cpuWrite(uint16_t addr, uint8_t data) {
    m_Ram[addr] = data;
  }

  uint8_t Gameboy::cpuRead(uint16_t addr, bool) {
    return m_Ram[addr];
  }

  void Gameboy::Update(double) {
    if (m_Run) {
      m_Cpu.Cycle(32);
    }
  }

  void Gameboy::HandleCPUExecution(const Events::ExecuteCPU &event) {
    m_Run = event.execute;
  }

  void Gameboy::HandleCPUStep(const Events::StepCPU &) {
    m_Cpu.Step();
  }

} // hijo