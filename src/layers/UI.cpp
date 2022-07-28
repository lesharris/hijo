#include "UI.h"

namespace hijo {
  void UI::OnAttach() {
// Hack to use opengl3 backend for imgui
    ImGui::CreateContext(nullptr);

    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Render fonts at half scale for sharper fonts.
    auto dpi = GetWindowScaleDPI();
    io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 16.0f * dpi.y);
    io.FontGlobalScale = 1.0f / dpi.y;

    auto glfwWindow = glfwGetCurrentContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    EventManager::Get().Attach<
        Events::KeyPressed,
        &::hijo::UI::HandleKeyPress
    >(this);
  }

  void UI::OnDetach() {
    EventManager::Get().DetachAll(this);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void UI::EndFrame() {
    ImGuiIO &io = ImGui::GetIO();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      auto currentContext = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(currentContext);
    }
  }

  void UI::Render() {
    auto &io = ImGui::GetIO();

    ImGui::ShowDemoWindow(&m_ShowDemo);
  }

  std::string UI::Name() const noexcept {
    return "ui";
  }

  void UI::HandleKeyPress(Events::KeyPressed &event) {

  }
} // hijo