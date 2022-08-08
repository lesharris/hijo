#pragma once

#include <raylib.h>

#include "common/common.h"
#include "core/Hijo.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "external/glfw/include/GLFW/glfw3.h"

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui_extra/imgui_memory_editor.h"

namespace hijo {

  class UI : public GameLayer {
  public:
    void OnAttach() override;

    void OnDetach() override;

    void Render() override;

    std::string Name() const noexcept override;

    bool RenderTarget() override { return false; }

    void BeginFrame() override;

    void EndFrame() override;

  private:
    void Viewport();

    void Disassembly();

    void Registers();

    void Tiles();

    void OAM();

    void CartridgeInfo();

    void CartridgeRuntime();

    void PPU();

  private:
    ImVec2 GetLargestSizeForViewport();

    ImVec2 GetCenteredPositionForViewport(ImVec2 &windowSize);

  private:
    void HandleKeyPress(Events::KeyPressed &event);

  private:
    Hijo &app = Hijo::Get();

    bool m_ShowDemo = false;
    bool m_ShowEmu = true;
    bool m_ShowRom = true;
    bool m_ShowWorkRam = true;
    bool m_ShowHighRam = true;
    bool m_ShowVRAM = true;
    bool m_ShowTiles = true;
    bool m_ShowDisassembly = true;
    bool m_ShowRegisters = true;
    bool m_ShowOAM = true;
    bool m_ShowPPU = true;
    bool m_ShowCartridge = true;
    bool m_ShowCartridgeRuntime = true;

    ImVec2 m_PreviousWindowSize{0, 0};
    ImVec2 m_PreviousMousePosition{0, 0};

    int32_t m_PrevScreenWidth;
    int32_t m_PrevScreenHeight;

    uint16_t m_PrevPC;

    struct RegItem {
      std::string label;
      uint16_t value;
      bool wide = false;
    };
  };

} // hijo

