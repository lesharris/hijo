#include "Emu.h"

#include <raylib.h>

#include "common/common.h"

namespace hijo {
  void Emu::OnAttach() {
    // m_GB.InsertCartridge("boilerplate.gb");
    //m_GB.InsertCartridge("boot_regs-dmgABC.gb");
    //m_GB.InsertCartridge("daa_test.gbc");
    //m_GB.InsertCartridge("reg_f.gb");
    //m_GB.InsertCartridge("daa.gb");

    //m_GB.InsertCartridge("mts/acceptance/intr_timing.gb");
    //m_GB.InsertCartridge("mts/acceptance/if_ie_registers.gb");
    //m_GB.InsertCartridge("mts/acceptance/jp_timing.gb");
    //m_GB.InsertCartridge("mts/acceptance/ld_hl_sp_e_timing.gb");
    //m_GB.InsertCartridge("mts/acceptance/rst_timing.gb");
    //m_GB.InsertCartridge("mts/acceptance/timer/rapid_toggle.gb");
    // m_GB.InsertCartridge("mts/acceptance/timer/tim00_div_trigger.gb");
//    m_GB.InsertCartridge("mts/acceptance/timer/tim01_div_trigger.gb");
    // m_GB.InsertCartridge("mts/acceptance/timer/tim10_div_trigger.gb");
    //m_GB.InsertCartridge("mts/acceptance/timer/tim11_div_trigger.gb");
    //m_GB.InsertCartridge("mts/acceptance/timer/tima_reload.gb");
    //m_GB.InsertCartridge("mts/acceptance/timer/tma_write_reloading.gb");
    // m_GB.InsertCartridge("mts/acceptance/timer/tima_write_reloading.gb");
    //m_GB.InsertCartridge("mts/acceptance/ppu/intr_2_0_timing.gb");
    //m_GB.InsertCartridge("mts/acceptance/ppu/intr_2_mode0_timing.gb");
//    m_GB.InsertCartridge("mts/acceptance/ppu/intr_2_oam_ok_timing.gb");
    // m_GB.InsertCartridge("mts/acceptance/ppu/lcdon_write_timing-GS.gb");
    // m_GB.InsertCartridge("mts/acceptance/ppu/stat_lyc_onoff.gb");
    //  m_GB.InsertCartridge("mts/acceptance/ppu/intr_1_2_timing-GS.gb");
    // m_GB.InsertCartridge("mts/acceptance/ppu/intr_2_mode0_timing_sprites.gb");
    // m_GB.InsertCartridge("mts/acceptance/ppu/intr_2_mode3_timing.gb");
    //m_GB.InsertCartridge("mts/acceptance/ppu/lcdon_timing-GS.gb");
    //m_GB.InsertCartridge("mts/acceptance/ppu/stat_irq_blocking.gb");
    // m_GB.InsertCartridge("mts/acceptance/ppu/vblank_stat_intr-GS.gb");
    //m_GB.InsertCartridge("mts/acceptance/oam_dma/sources-GS.gb");
    //m_GB.InsertCartridge("mts/acceptance/oam_dma/reg_read.gb");
    //m_GB.InsertCartridge("mts/emulator-only/mbc1/ram_256kb.gb");

    // m_GB.InsertCartridge("mem_timing.gb");
    //m_GB.InsertCartridge("instr_timing.gb");

    //     m_GB.InsertCartridge("mts/manual-only/sprite_priority.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/oam_dma/reg_read.gb"); // PASSED
    //    m_GB.InsertCartridge("mts/acceptance/oam_dma/basic.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/timer/tim11.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/timer/tim10.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/timer/tim01.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/timer/tim00.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/timer/div_write.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/oam_dma_timing.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/oam_dma_restart.gb"); // PASSED
    //    m_GB.InsertCartridge("mts/acceptance/instr/daa.gb"); // PASSED
    //     m_GB.InsertCartridge("mts/acceptance/div_timing.gb"); // PASSED
    // m_GB.InsertCartridge("mts/acceptance/ret_timing.gb"); // PASSED
    //m_GB.InsertCartridge("mts/acceptance/push_timing.gb");
    // m_GB.InsertCartridge("mts/acceptance/pop_timing.gb");//PASSED

    // m_GB.InsertCartridge("01-special.gb"); // PASSED
    //m_GB.InsertCartridge("02-interrupts.gb"); // PASSED
    //m_GB.InsertCartridge("03-op sp,hl.gb"); // PASSED
    //m_GB.InsertCartridge("04-op r,imm.gb"); // PASSED
    //m_GB.InsertCartridge("05-op rp.gb"); // PASSED
    //m_GB.InsertCartridge("06-ld r,r.gb"); // PASSED
    //m_GB.InsertCartridge("07-jr,jp,call,ret,rst.gb"); // PASSED
    //m_GB.InsertCartridge("08-misc instrs.gb"); // PASSED
    //m_GB.InsertCartridge("09-op r,r.gb"); // PASSED
    //m_GB.InsertCartridge("10-bit ops.gb"); // PASSED
    //m_GB.InsertCartridge("11-op a,(hl).gb"); // PASSED

    //m_GB.InsertCartridge("cpu_instrs.gb"); // PASSED

    //m_GB.InsertCartridge("linksawakening.gb");
    //m_GB.InsertCartridge("kirbysdreamland.gb");
    //m_GB.InsertCartridge("supermarioland2.gb");
    //m_GB.InsertCartridge("supermarioland.gb");
    //m_GB.InsertCartridge("tetris.gb");
    //m_GB.InsertCartridge("drmario.gb");
    //m_GB.InsertCartridge("alleyway.gb");
    //m_GB.InsertCartridge("dmg-acid2.gb"); // PASSED

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
    return std::string("emu");
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