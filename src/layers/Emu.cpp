#include "Emu.h"

#include <raylib.h>

#include "common/common.h"

namespace hijo {
  void Emu::OnAttach() {
    //m_GB.InsertCartridge("boot_regs-dmgABC.gb");
    //m_GB.InsertCartridge("daa_test.gbc");
    //m_GB.InsertCartridge("reg_f.gb");
    // m_GB.InsertCartridge("daa.gb");

    //  m_GB.InsertCartridge("instr_timing.gb");
    //m_GB.InsertCartridge("02-interrupts.gb");
    // m_GB.InsertCartridge("07-jr,jp,call,ret,rst.gb");

    //m_GB.InsertCartridge("01-special.gb"); // Passed
    //m_GB.InsertCartridge("03-op sp,hl.gb"); // Passed
    //m_GB.InsertCartridge("04-op r,imm.gb"); // Passed
    //m_GB.InsertCartridge("05-op rp.gb"); // Passed
    m_GB.InsertCartridge("06-ld r,r.gb"); // Passed
    //m_GB.InsertCartridge("08-misc instrs.gb"); // Passed
    //m_GB.InsertCartridge("09-op r,r.gb"); // Passed
    //m_GB.InsertCartridge("10-bit ops.gb"); // Passed
    //m_GB.InsertCartridge("11-op a,(hl).gb"); // Passed

    //m_GB.InsertCartridge("cpu_instrs.gb");

    //m_GB.InsertCartridge("tetris.gb");
    //m_GB.InsertCartridge("drmario.gb");
    //m_GB.InsertCartridge("alleyway.gb");
    //m_GB.InsertCartridge("dmg-acid2.gb");

    EventManager::Get().Attach<
        Events::KeyPressed,
        &Emu::HandleKeyPress
    >(this);

    EventManager::Get().Attach<
        Events::InputAction,
        &Emu::HandleAction
    >(this);
  }

  void Emu::OnDetach() {
    EventManager::Get().DetachAll(this);
  }

  void Emu::Update(double timestep) {
    m_GB.Update(timestep);
  }

  void Emu::Render() {
    auto scw = app.ScreenWidth();
    const auto &buffer = m_GB.VideoBuffer();
    uint8_t bufferHeight = 144;
    uint8_t bufferWidth = 160;

    auto scale = scw / bufferWidth;

    for (auto y = 0; y < bufferHeight; y++) {
      for (auto x = 0; x < bufferWidth; x++) {
        DrawRectangle(x * scale, y * scale,
                      scale, scale,
                      buffer[(y * bufferWidth) + x]);
      }
    }
  }

  std::string Emu::Name() const noexcept {
    return std::string("level");
  }

  void Emu::HandleKeyPress(Events::KeyPressed &event) {
    if (event.handled) {
      return;
    }

    bool handled = true;

    switch (event.key) {
      default:
        handled = false;
        break;
    }

    event.handled = handled;
  }

  void Emu::HandleAction(const Events::InputAction &event) {
    switch (event.type) {

      default:
        break;
    }
  }

  void Emu::RenderTexture() {
  }
} // hijo