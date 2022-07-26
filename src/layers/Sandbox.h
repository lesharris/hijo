#pragma once

#include "common/common.h"
#include "core/Hijo.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "system/Gameboy.h"

namespace hijo {

  class Sandbox : public GameLayer {
  public:
    void OnAttach() override;

    void OnDetach() override;

    void Update(double timestep) override;

    void Render() override;

    void RenderTexture() override;

    std::string Name() const noexcept override;

    bool RenderTarget() override { return true; }

  private:
    void HandleKeyPress(Events::KeyPressed &event);

    void HandleAction(const Events::InputAction &event);

  private:
    Hijo &app = Hijo::Get();

    Gameboy m_GB;
  };

} // hijo

