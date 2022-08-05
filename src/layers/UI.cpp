#include "UI.h"
#include "imgui_internal.h"

#include "system/Gameboy.h"
#include "cpu/Interrupts.h"
#include "display/PPU.h"

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
        if (ImGui::BeginMenu("CPU")) {
          ImGui::MenuItem("Registers", NULL, &m_ShowRegisters);
          ImGui::MenuItem("Disassembly", NULL, &m_ShowDisassembly);
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("RAM")) {
          ImGui::MenuItem("ROM Viewer", NULL, &m_ShowRom);
          ImGui::MenuItem("Work RAM Viewer", NULL, &m_ShowWorkRam);
          ImGui::MenuItem("High RAM Viewer", NULL, &m_ShowHighRam);
          ImGui::MenuItem("VRAM Viewer", NULL, &m_ShowVRAM);
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graphics")) {
          ImGui::MenuItem("Tile Viewer", NULL, &m_ShowTiles);
          ImGui::MenuItem("OAM Viewer", NULL, &m_ShowOAM);
          ImGui::EndMenu();
        }

        ImGui::Separator();
        ImGui::MenuItem("Cartridge Info", NULL, &m_ShowCartridge);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemo);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    Gameboy *gb = app.System<Gameboy>();
    auto &cpu = gb->m_Cpu;
    auto &regs = cpu.regs;

    if (m_ShowRom) {
      static MemoryEditor romViewer;

      romViewer.HighlightMin = regs.pc;
      romViewer.HighlightMax = regs.pc;

      if (gb->m_Cartridge)
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

    if (m_ShowTiles) {
      Tiles();
    }

    if (m_ShowOAM) {
      OAM();
    }

    if (m_ShowCartridge) {
      CartridgeInfo();
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


  void UI::Tiles() {
    if (!ImGui::Begin("Tiles", &m_ShowTiles)) {
      ImGui::End();
    } else {
      auto &texture = Hijo::Get().GetTileTexture();
      ImGui::Image(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                   {static_cast<float>(texture.texture.width), static_cast<float>(texture.texture.height)},
                   {0, 1}, {1, 0});

      ImGui::End();
    }
  }

  void UI::Viewport() {
    static bool firstRun = true;
    static bool second = true;

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

      if (firstRun || second || windowWidth != m_PrevScreenWidth || windowHeight != m_PrevScreenHeight) {
        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;

        firstRun = false;
        second = false;

        EventManager::Dispatcher().trigger<Events::ViewportResized>({size.x, size.y});
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

    if (cpu.m_Disassembly.empty()) {
      cpu.Disassemble(0, (64 * 1024) - 1);
    }

    if (!ImGui::Begin("Disassembly", &m_ShowDisassembly)) {
      ImGui::End();
    } else {
      if (ImGui::BeginTable("disa", 3, ImGuiTableFlags_ScrollY |
                                       ImGuiTableFlags_BordersOuterH |
                                       ImGuiTableFlags_BordersOuterV |
                                       ImGuiTableFlags_RowBg)) {
        auto &lines = cpu.m_Disassembly;

        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);
        // ImGui::TableSetupColumn("Addr. Mode", ImGuiTableColumnFlags_WidthFixed, 40.0f);

        ImGuiListClipper clipper;
        clipper.Begin(lines.size());

        while (clipper.Step()) {
          for (auto item = clipper.DisplayStart; item < clipper.DisplayEnd; item++) {
            auto &regs = cpu.regs;

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

            //ImGui::TableSetColumnIndex(3);
            //ImGui::TextUnformatted(fmt::format("[{}]", line.mode.c_str()).c_str());
          }
        }

        auto &regs = cpu.regs;


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
    auto &regs = cpu.regs;

    if (!ImGui::Begin("Registers", &m_ShowRegisters)) {
      ImGui::End();
    } else {
      bool Z = cpu.CPU_FLAG_Z();
      bool N = cpu.CPU_FLAG_N();
      bool H = cpu.CPU_FLAG_H();
      bool C = cpu.CPU_FLAG_C();

      uint8_t IF = cpu.IntFlags();
      uint8_t IE = cpu.IERegister();

      bool IF_VBlank = BIT(IF, 0);
      bool IF_LCDStat = BIT(IF, 1);
      bool IF_Timer = BIT(IF, 2);
      bool IF_Serial = BIT(IF, 3);
      bool IF_Joypad = BIT(IF, 4);

      bool IE_VBlank = BIT(IE, 0);
      bool IE_LCDStat = BIT(IE, 1);
      bool IE_Timer = BIT(IE, 2);
      bool IE_Serial = BIT(IE, 3);
      bool IE_Joypad = BIT(IE, 4);

      bool MasterInterrupt = cpu.m_InterruptMasterEnabled;

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
          {"AF", cpu.reverse(*((uint16_t *) &regs.a)), true},
          {"BC", cpu.reverse(*((uint16_t *) &regs.b)), true},
          {"DE", cpu.reverse(*((uint16_t *) &regs.d)), true},
          {"HL", cpu.reverse(*((uint16_t *) &regs.h)), true},
          {"A",  regs.a},
          {"B",  regs.b},
          {"C",  regs.c},
          {"D",  regs.d},
          {"E",  regs.e},
          {"H",  regs.h},
          {"L",  regs.l},
          {"SP", regs.sp,                              true},
          {"PC", regs.pc,                              true}
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
      ImGui::Checkbox("Interrupt Master Enable", &MasterInterrupt);
      ImGui::Separator();
      ImGui::TextUnformatted(fmt::format("IF: {:08b}", cpu.IntFlags()).c_str());
      ImGui::BeginGroup();
      ImGui::Checkbox("VBlank", &IF_VBlank);
      ImGui::SameLine();
      ImGui::Checkbox("LCD Stat", &IF_LCDStat);
      ImGui::SameLine();
      ImGui::Checkbox("Timer", &IF_Timer);
      ImGui::SameLine();
      ImGui::Checkbox("Serial", &IF_Serial);
      ImGui::SameLine();
      ImGui::Checkbox("Joypad", &IF_Joypad);
      ImGui::EndGroup();
      ImGui::Separator();
      ImGui::TextUnformatted(fmt::format("IE: {:08b}", cpu.IERegister()).c_str());
      ImGui::BeginGroup();
      ImGui::Checkbox("VBlank", &IE_VBlank);
      ImGui::SameLine();
      ImGui::Checkbox("LCD Stat", &IE_LCDStat);
      ImGui::SameLine();
      ImGui::Checkbox("Timer", &IE_Timer);
      ImGui::SameLine();
      ImGui::Checkbox("Serial", &IE_Serial);
      ImGui::SameLine();
      ImGui::Checkbox("Joypad", &IE_Joypad);
      ImGui::EndGroup();

      // Interrupts

      ImGui::End();
    }
  }

  void UI::OAM() {
    Gameboy *gb = app.System<Gameboy>();
    auto &oam = gb->m_PPU.m_OAMRam;
    auto &texture = Hijo::Get().GetPackedTileTexture();

    if (!ImGui::Begin("OAM", &m_ShowOAM)) {
      ImGui::End();
    } else {
      ImGui::BeginTable("oam", 8, ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_BordersOuterH |
                                  ImGuiTableFlags_BordersOuterV |
                                  ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("#");
      ImGui::TableSetupColumn("X");
      ImGui::TableSetupColumn("Y");
      ImGui::TableSetupColumn("Tile");
      ImGui::TableSetupColumn("Pal#");
      ImGui::TableSetupColumn("XFlip");
      ImGui::TableSetupColumn("YFlip");
      ImGui::TableSetupColumn("BGP");
      ImGui::TableSetupScrollFreeze(8, 1);

      ImGui::TableHeadersRow();

      uint8_t count = 0;

      for (size_t i = 0; i < 40; i++) {
        const auto &entry = oam[i];

        int colCount = (int) (texture.texture.width / 32.0f);
        int tx = entry.tile % colCount;
        int ty = (int) ((float) entry.tile / (float) colCount);

        float x = tx * 32.0f;
        float y = ty * 32.0f;

        ImVec2 uv0{x / texture.texture.width, y / texture.texture.height};
        ImVec2 uv1{(x + 32.0f) / texture.texture.width, (y + 32.0f) / texture.texture.height};

        uv0.y = -uv0.y;
        uv1.y = -uv1.y;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", count++);
        ImGui::TableNextColumn();
        ImGui::Text("%d", entry.x);
        ImGui::TableNextColumn();
        ImGui::Text("%d", entry.y);
        ImGui::TableNextColumn();
        ImGui::Text("%d", entry.tile);
        ImGui::Image(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                     {32.0f, 32.0f},
                     uv0, uv1);
        ImGui::TableNextColumn();
        ImGui::Text("%d", PPU::OAMEntryPaletteNumber(entry));
        ImGui::TableNextColumn();
        ImGui::Text("%s", PPU::OAMEntryXFlip(entry) ? "Y" : "N");
        ImGui::TableNextColumn();
        ImGui::Text("%s", PPU::OAMEntryYFlip(entry) ? "Y" : "N");
        ImGui::TableNextColumn();
        ImGui::Text("%s", PPU::OAMEntryBackgroundPriority(entry) ? "Y" : "N");
      }

      ImGui::EndTable();
      ImGui::End();
    }
  }

  void UI::CartridgeInfo() {
    auto &bus = Gameboy::Get();
    auto cartridge = bus.m_Cartridge;

    if (!ImGui::Begin("Cartridge", &m_ShowCartridge)) {
      ImGui::End();
    } else {
      if (!cartridge) {
        ImGui::Text("No cartridge loaded!");
      } else {
        const auto &header = cartridge->Header();
        const auto &statLines = cartridge->m_Mapper->GetStats();

        ImGui::BeginTable("cinfo", 2, ImGuiTableFlags_RowBg);
        ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Name");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", header.title.c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Licensee");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", header.licensee.c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Mapper");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", header.mapperInfo.label.c_str());

        ImGui::EndTable();

        ImGui::Separator();

        bool hasRam = header.mapperInfo.hasRam;
        bool hasTimer = header.mapperInfo.hasTimer;
        bool hasBattery = header.mapperInfo.hasBattery;
        bool hasRumble = header.mapperInfo.hasRumble;

        ImGui::Text("Cartridge Features:");
        ImGui::Checkbox("Ram", &hasRam);
        ImGui::SameLine();
        ImGui::Checkbox("Battery", &hasBattery);
        ImGui::SameLine();
        ImGui::Checkbox("Timer", &hasTimer);
        ImGui::SameLine();
        ImGui::Checkbox("Rumble", &hasRumble);

        ImGui::Separator();

        ImGui::BeginTable("cinfo2", 2, ImGuiTableFlags_RowBg);
        ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

        std::string cgbFlag = "";

        switch (header.CGBFlag) {
          case 0x80:
            cgbFlag = "Enhanced";
            break;

          case 0xC0:
            cgbFlag = "Required";
            break;

          default:
            cgbFlag = "None";
            break;
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("GB Color Support");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", cgbFlag.c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Super Gameboy");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", header.sgbFlag == 0x3 ? "Supported" : "Not Supported");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Region Code");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", header.destinationCode == 0 ? "Japan/World" : "World");

        ImGui::EndTable();

        ImGui::Separator();

        ImGui::BeginTable("cinfo3", 2, ImGuiTableFlags_RowBg);
        ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rom Size");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s [%d banks]", header.romInfo.label.c_str(), header.romInfo.romBankCount);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Ram Size");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s [%d banks]", header.ramInfo.ramBankCount > 0
                                     ? header.ramInfo.label.c_str()
                                     : "None",
                    header.ramInfo.ramBankCount);

        ImGui::EndTable();

        ImGui::Separator();

        ImGui::BeginTable("cinfo4", 2, ImGuiTableFlags_RowBg);
        ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rom Version");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("0x%02X", header.maskRomVersion);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Header Checksum");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("0x%02X [%s]", header.headerChecksum, header.headerChecksumPassed ? "Passed" : "Failed");

        ImGui::EndTable();

        ImGui::Separator();

        ImGui::BeginTable("cinfo5", 2, ImGuiTableFlags_RowBg);
        ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

        for (const auto &line: statLines) {
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%s", line.label.c_str());
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", line.data.c_str());
        }

        ImGui::EndTable();
      }
      ImGui::End();
    }
  }

} // hijo