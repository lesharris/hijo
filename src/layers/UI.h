#pragma once

#include "common/common.h"
#include "core/Hijo.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "external/glfw/include/GLFW/glfw3.h"

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"
#include "external/imgui/backends/imgui_impl_glfw.h"

namespace hijo {

  class UI : public GameLayer {
  public:
    void OnAttach() override;

    void OnDetach() override;

    void Render() override;

    std::string Name() const noexcept override;

    bool RenderTarget() override { return true; }

    void BeginFrame() override;

    void EndFrame() override;

  private:
    void HandleKeyPress(Events::KeyPressed &event);

  private:
    Hijo &app = Hijo::Get();

    bool m_ShowDemo = true;
  };

} // hijo

