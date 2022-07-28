#include "Sandbox.h"

#include <raylib.h>

#include "common/common.h"

namespace hijo {
  void Sandbox::OnAttach() {
    EventManager::Get().Attach<
        Events::KeyPressed,
        &Sandbox::HandleKeyPress
    >(this);

    EventManager::Get().Attach<
        Events::InputAction,
        &Sandbox::HandleAction
    >(this);
  }

  void Sandbox::OnDetach() {
    EventManager::Get().DetachAll(this);
  }

  void Sandbox::Update(double timestep) {
    m_GB.Update(timestep);
  }

  void Sandbox::Render() {
    DrawText("Hello!", 10, 10, 24, WHITE);
  }

  std::string Sandbox::Name() const noexcept {
    return std::string("level");
  }

  void Sandbox::HandleKeyPress(Events::KeyPressed &event) {
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

  void Sandbox::HandleAction(const Events::InputAction &event) {
    switch (event.type) {

      default:
        break;
    }
  }

  void Sandbox::RenderTexture() {
  }
} // hijo