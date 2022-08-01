#include "Emu.h"

#include <raylib.h>

#include "common/common.h"

#include <iostream>

namespace hijo {
  void Emu::OnAttach() {
    m_GB.InsertCartridge("cpu_instrs.gb");
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