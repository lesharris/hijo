#include "UI.h"
#include "imgui_internal.h"

#include "system/Gameboy.h"
#include "cpu/Interrupts.h"

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

    m_PrevPC = 1;

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
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    if (opt_fullscreen) {
      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
      dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Dockspace", nullptr, window_flags);

    if (!opt_padding)
      ImGui::PopStyleVar();

    if (opt_fullscreen)
      ImGui::PopStyleVar(2);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Quit", NULL)) {
          EventManager::Dispatcher().trigger<Events::WantQuit>();
        }

        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Tools")) {
        ImGui::MenuItem("Registers", NULL, &m_ShowRegisters);
        ImGui::MenuItem("ROM Viewer", NULL, &m_ShowRom);
        ImGui::MenuItem("Work RAM Viewer", NULL, &m_ShowWorkRam);
        ImGui::MenuItem("High RAM Viewer", NULL, &m_ShowHighRam);
        ImGui::MenuItem("Disassembly", NULL, &m_ShowDisassembly);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemo);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    Gameboy *gb = app.System<Gameboy>();
    auto &cpu = gb->m_Cpu;
    auto &regs = cpu.GetRegisters();

    if (m_ShowRom) {
      static MemoryEditor romViewer;

      romViewer.HighlightMin = regs.pc;
      romViewer.HighlightMax = regs.pc;

      romViewer.DrawWindow("ROM", &gb->m_Cartridge->Data(), 32 * 1024);
    }

    if (m_ShowWorkRam) {
      static MemoryEditor wramEditor;

      wramEditor.DrawWindow("WRAM", &gb->m_WorkRam, 8 * 1024, 0xC000);
    }

    if (m_ShowHighRam) {
      static MemoryEditor hramEditor;

      hramEditor.DrawWindow("HRAM", &gb->m_HighRam, 127, 0xFF80);
    }

    if (m_ShowVRAM) {
      static MemoryEditor vramEditor;

      vramEditor.DrawWindow("VRAM", &gb->m_PPU.m_VideoRam, 8 * 1024, 0x8000);
    }

    if (m_ShowDisassembly) {
      Disassembly();
    }

    if (m_ShowRegisters) {
      Registers();
    }

    if (m_ShowDemo) {
      ImGui::ShowDemoWindow(&m_ShowDemo);
    }

    Viewport();

    ImGui::End();
  }

  std::string UI::Name() const noexcept {
    return "ui";
  }

  /*
   * Event Handlers
   */
  void UI::HandleKeyPress(Events::KeyPressed &) {

  }

  /*
   * Utilities
   */
  ImVec2 UI::GetLargestSizeForViewport() {
    ImVec2 size = ImGui::GetContentRegionAvail();

    size.x -= ImGui::GetScrollX();
    size.y -= ImGui::GetScrollY();

    float aspectWidth = size.x;
    float aspectHeight = aspectWidth / (10.0f / 9.0f);

    if (aspectHeight > size.y) {
      aspectHeight = size.y;
      aspectWidth = aspectHeight * (10.0f / 9.0f);
    }

    return {aspectWidth, aspectHeight};
  }

  ImVec2 UI::GetCenteredPositionForViewport(ImVec2 &aspectSize) {
    ImVec2 size = ImGui::GetContentRegionAvail();

    size.x -= ImGui::GetScrollX();
    size.y -= ImGui::GetScrollY();

    float viewportX = (size.x / 2.0f) - (aspectSize.x / 2.0);
    float viewportY = (size.y / 2.0f) - (aspectSize.y / 2.0);

    return {viewportX + ImGui::GetCursorPosX(), viewportY + ImGui::GetCursorPosY()};
  }

  /*
   * Windows
   */
  void UI::Viewport() {
    static bool firstRun = true;

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

    // Render our Emu Viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::SetNextWindowClass(&window_class);

    if (!ImGui::Begin("Screen", &m_ShowEmu,
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse |
                      ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoDecoration)) {
      ImGui::End();
    } else {
      auto windowWidth = GetScreenWidth();
      auto windowHeight = GetScreenHeight();

      auto size = ImGui::GetContentRegionAvail();
      auto contentMin = ImGui::GetWindowContentRegionMin();
      auto windowPos = ImGui::GetWindowPos();
      auto mousePos = ImGui::GetMousePos();

      auto topLeft = ImVec2{windowPos.x + contentMin.x, windowPos.y + contentMin.y};
      auto bottomRight = ImVec2{topLeft.x + size.x, topLeft.y + size.y};

      ImVec2 emuWindowSize = GetLargestSizeForViewport();
      ImVec2 emuWindowPos = GetCenteredPositionForViewport(emuWindowSize);

      ImGui::SetCursorPos(emuWindowPos);

      // Is mouse is over the viewport window...
      if (ImGui::IsMouseHoveringRect(topLeft, bottomRight)) {
        // Transform from screen to window local coords
        ImVec2 position = {mousePos.x - topLeft.x, mousePos.y - topLeft.y};

        EventManager::Dispatcher().trigger<Events::UIMouseMove>({position.x, position.y});
      }

      if (firstRun || windowWidth != m_PrevScreenWidth || windowHeight != m_PrevScreenHeight) {
        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;

        firstRun = false;
        
        EventManager::Dispatcher().enqueue<Events::ViewportResized>({size.x, size.y});
      } else {
        auto &texture = Hijo::Get().GetRenderTexture();
        ImGui::Image(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                     emuWindowSize,
                     {0, 1}, {1, 0});

        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;
      }

      ImGui::End();
    }

    ImGui::PopStyleVar(3);
  }

  void UI::Disassembly() {
    Gameboy *gb = app.System<Gameboy>();
    auto &cpu = gb->m_Cpu;

    if (cpu.Disassembly().empty()) {
      cpu.Disassemble(0, (64 * 1024) - 1);
    }

    if (!ImGui::Begin("Disassembly", &m_ShowDisassembly)) {
      ImGui::End();
    } else {
      if (ImGui::BeginTable("disa", 4, ImGuiTableFlags_ScrollY |
                                       ImGuiTableFlags_BordersOuterH |
                                       ImGuiTableFlags_BordersOuterV |
                                       ImGuiTableFlags_RowBg)) {
        auto &lines = cpu.Disassembly();

        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Addr. Mode", ImGuiTableColumnFlags_WidthFixed, 40.0f);

        ImGuiListClipper clipper;
        clipper.Begin(lines.size());

        while (clipper.Step()) {
          for (auto item = clipper.DisplayStart; item < clipper.DisplayEnd; item++) {
            auto &regs = cpu.GetRegisters();

            auto &line = lines[item];
            ImGui::TableNextRow();

            if (line.addr == regs.pc) {
              ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.18f, 0.47f, 0.59f, 0.65f));
              ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, row_bg_color);
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(fmt::format("${:04X}", line.addr).c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(fmt::format("{}", line.bytes).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(line.text.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(fmt::format("[{}]", line.mode.c_str()).c_str());
          }
        }

        auto &regs = cpu.GetRegisters();


        if (regs.pc != m_PrevPC) {
          auto it = std::find_if(
              lines.begin(),
              lines.end(),
              [regs](const auto &line) {
                return line.addr == regs.pc;
              });

          if (it != std::end(lines)) {
            auto size = ImGui::GetWindowSize();
            ImGui::SetScrollY(
                (clipper.ItemsHeight * it->index - (size.y / 2) - (clipper.ItemsHeight / clipper.ItemsCount)));
            m_PrevPC = regs.pc;
          }
        }

        ImGui::EndTable();
      }

      ImGui::End();
    }
  }

  void UI::Registers() {
    Gameboy *gb = app.System<Gameboy>();
    auto &cpu = gb->m_Cpu;
    auto &regs = cpu.GetRegisters();

    if (!ImGui::Begin("Registers", &m_ShowRegisters)) {
      ImGui::End();
    } else {
      bool Z = cpu.Zero();
      bool N = cpu.Negative();
      bool H = cpu.HalfCarry();
      bool C = cpu.Carry();

      bool VBlank = Interrupts::VBlank(cpu);
      bool LCDStat = Interrupts::LCDStat(cpu);
      bool Serial = Interrupts::Serial(cpu);
      bool Timer = Interrupts::Timer(cpu);
      bool Joypad = Interrupts::Joypad(cpu);

      bool MasterInterrupt = cpu.m_InterruptsEnabled;

      ImGui::BeginTable("flagsbtnscycles", 2);
      ImGui::TableSetupColumn("btns");
      ImGui::TableSetupColumn("cycles");

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      if (ImGui::Button(gb->Running() ? "Pause" : "Run")) {
        EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({!gb->Running()});
      }

      ImGui::SameLine();

      if (!gb->Running()) {
        if (ImGui::Button("Step")) {
          EventManager::Dispatcher().enqueue<Events::StepCPU>({});
        }
      }

      ImGui::SameLine();

      if (!gb->Running()) {
        static uint16_t addr = 0;
        ImGui::InputScalar("Run to", ImGuiDataType_U16, &addr, NULL, NULL, "%04X");
        if (ImGui::Button("Go")) {
          EventManager::Dispatcher().enqueue<Events::ExecuteUntil>({addr});
        }
      }

      ImGui::TableSetColumnIndex(1);
      auto cycleInfo = fmt::format("Total Cycles: {}", gb->Cycles());
      auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(cycleInfo.c_str()).x
                   - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
      if (posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
      ImGui::TextUnformatted(cycleInfo.c_str());

      ImGui::EndTable();

      ImGui::Separator();

      std::vector<RegItem> items = {
          {"AF", regs.af, true},
          {"BC", regs.bc, true},
          {"DE", regs.de, true},
          {"HL", regs.hl, true},
          {"A",  regs.a},
          {"B",  regs.b},
          {"C",  regs.c},
          {"D",  regs.d},
          {"E",  regs.e},
          {"H",  regs.h},
          {"L",  regs.l},
          {"SP", regs.sp, true},
          {"PC", regs.pc, true},
          {"IE", regs.ie}
      };

      ImGui::BeginTable("regs", 4, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("r", ImGuiTableColumnFlags_WidthFixed, 50.0f);
      ImGui::TableSetupColumn("rh", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableSetupColumn("rd", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableSetupColumn("rb", ImGuiTableColumnFlags_None);

      for (const auto &item: items) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(item.label.c_str());
        ImGui::TableSetColumnIndex(1);
        if (item.wide) {
          ImGui::TextUnformatted(fmt::format("${:04X}", item.value).c_str());
        } else {
          ImGui::TextUnformatted(fmt::format("${:02X}", item.value).c_str());
        }
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(fmt::format("{}", item.value).c_str());
        ImGui::TableSetColumnIndex(3);
        if (item.wide) {
          uint8_t highByte = (item.value & 0xFF00) >> 8;
          uint8_t lowByte = item.value & 0xFF;
          uint8_t highByteHighNib = (highByte & 0xF0) >> 4;
          uint8_t highByteLowNib = highByte & 0xF;
          uint8_t lowByteHighNib = (lowByte & 0xF0) >> 4;
          uint8_t lowByteLowNib = lowByte & 0xF;

          ImGui::TextUnformatted(fmt::format("{:04b} {:04b} {:04b} {:04b}",
                                             highByteHighNib,
                                             highByteLowNib,
                                             lowByteHighNib,
                                             lowByteLowNib).c_str());
        } else {
          uint8_t highNib = (item.value & 0xF0) >> 4;
          uint8_t lowNib = item.value & 0xF;

          ImGui::TextUnformatted(fmt::format("{:04b} {:04b}", highNib, lowNib).c_str());
        }
      }

      ImGui::EndTable();
      ImGui::Separator();

      // Flags
      ImGui::BeginGroup();
      ImGui::Checkbox("Z", &Z);
      ImGui::SameLine();
      ImGui::Checkbox("N", &N);
      ImGui::SameLine();
      ImGui::Checkbox("H", &H);
      ImGui::SameLine();
      ImGui::Checkbox("C", &C);
      ImGui::EndGroup();
      ImGui::Separator();
      ImGui::Checkbox("Master Interrupt", &MasterInterrupt);
      ImGui::Checkbox("VBlank", &VBlank);
      ImGui::Checkbox("LCD Stat", &LCDStat);
      ImGui::Checkbox("Timer", &Timer);
      ImGui::Checkbox("Serial", &Serial);
      ImGui::Checkbox("Joypad", &Joypad);

      // Interrupts

      ImGui::End();
    }
  }
} // hijo