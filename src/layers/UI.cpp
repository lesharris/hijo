#include "UI.h"
#include "imgui_internal.h"

#include <nfd.h>

#include "system/Gameboy.h"
#include "cpu/Interrupts.h"
#include "display/PPU.h"
#include "display/LCD.h"

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
        if (ImGui::MenuItem("Open ROM...", NULL)) {
          nfdchar_t *outPath = nullptr;
          nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

          switch (result) {
            case NFD_OKAY: {
              std::string path{outPath};
              EventManager::Dispatcher().trigger(Events::LoadROM{path});
              delete outPath;
            }
              break;
            case NFD_CANCEL:
              break;
            case NFD_ERROR:
              spdlog::get("console")->error("{}", NFD_GetError());
              break;
          }
        }

        ImGui::Separator();
        
        if (ImGui::MenuItem("Unload ROM")) {
          EventManager::Dispatcher().trigger(Events::UnloadROM{});
        }

        if (ImGui::MenuItem("Reset")) {
          EventManager::Dispatcher().trigger(Events::Reset{});
        }

        ImGui::Separator();
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
          ImGui::MenuItem("PPU", NULL, &m_ShowPPU);
          ImGui::MenuItem("Tile Viewer", NULL, &m_ShowTiles);
          ImGui::MenuItem("OAM Viewer", NULL, &m_ShowOAM);
          ImGui::EndMenu();
        }

        ImGui::Separator();
        if (ImGui::BeginMenu("Cartridge")) {
          ImGui::MenuItem("Cartridge Info", NULL, &m_ShowCartridge);
          ImGui::MenuItem("Cartridge Runtime", NULL, &m_ShowCartridgeRuntime);
          ImGui::EndMenu();
        }

        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemo);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    Gameboy *gb = app.System<Gameboy>();

    // Display Windows
    {
      if (m_ShowRom) {
        static MemoryEditor romViewer;


        if (gb->m_Cartridge) {
          romViewer.DrawWindow("ROM", &gb->m_Cartridge->Data(), 32 * 1024);
        }
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

      if (m_ShowCartridgeRuntime) {
        CartridgeRuntime();
      }

      if (m_ShowPPU) {
        PPU();
      }

      Viewport();
    }

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

      if (windowWidth != m_PrevScreenWidth || windowHeight != m_PrevScreenHeight) {
        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;

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


    if (cpu.m_Disassembly.empty() && gb->m_Cartridge) {
      cpu.Disassemble(0, (64 * 1024) - 1);
    }

    if (!ImGui::Begin("Disassembly", &m_ShowDisassembly)) {
      ImGui::End();
    } else {

      if (!gb->m_Cartridge) {
        ImGui::Text("No Cartridge Loaded");
        ImGui::End();
        return;
      }

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

      bool IF_VBlank = Bit(IF, 0);
      bool IF_LCDStat = Bit(IF, 1);
      bool IF_Timer = Bit(IF, 2);
      bool IF_Serial = Bit(IF, 3);
      bool IF_Joypad = Bit(IF, 4);

      bool IE_VBlank = Bit(IE, 0);
      bool IE_LCDStat = Bit(IE, 1);
      bool IE_Timer = Bit(IE, 2);
      bool IE_Serial = Bit(IE, 3);
      bool IE_Joypad = Bit(IE, 4);

      bool MasterInterrupt = cpu.m_InterruptMasterEnabled;

      ImGui::BeginTable("flagsbtnscycles", 2);
      ImGui::TableSetupColumn("btns");
      ImGui::TableSetupColumn("cycles");

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      if (gb->m_Cartridge) {
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
      }

      ImGui::TableSetColumnIndex(1);
      auto cycleInfo = fmt::format("T Cycles per Frame: {}", gb->Cycles());
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
        ImGui::Text("No Cartridge Loaded");
      } else {
        const auto &header = cartridge->Header();

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
      }
      ImGui::End();
    }
  }

  void UI::CartridgeRuntime() {
    if (!ImGui::Begin("Cart. Runtime", &m_ShowCartridgeRuntime)) {
      ImGui::End();
    } else {
      auto &bus = Gameboy::Get();
      auto cartridge = bus.m_Cartridge;

      if (!cartridge) {
        ImGui::Text("No Cartridge Loaded");
        ImGui::End();
        return;
      }

      const auto &statLines = cartridge->m_Mapper->GetStats();

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

      ImGui::End();
    }
  }

  void UI::PPU() {
    auto &bus = Gameboy::Get();
    auto &ppu = bus.m_PPU;
    auto &lcd = LCD::Get();

    auto lcdMode = [](LCD::Mode mode) {
      switch (mode) {
        case LCD::Mode::HBlank:
          return "HBlank";
        case LCD::Mode::VBlank:
          return "VBlank";
        case LCD::Mode::OAM:
          return "OAM";
        case LCD::Mode::XFER:
          return "XFER";
      }
    };

    if (!ImGui::Begin("PPU", &m_ShowPPU)) {
      ImGui::End();
    } else {
      if (ImGui::BeginTabBar("ptab", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("LCD")) {
          ImGui::TextUnformatted(fmt::format("LCD Control {:04b} {:04b}",
                                             (lcd.regs.LCDC & 0xF0) >> 4,
                                             lcd.regs.LCDC & 0xF).c_str());

          bool LCDC_Enable = Bit(lcd.regs.LCDC, 7);
          bool LCDC_WinMapArea = Bit(lcd.regs.LCDC, 6);
          bool LCDC_WindowEnable = Bit(lcd.regs.LCDC, 5);
          bool LCDC_TileDataArea = Bit(lcd.regs.LCDC, 4);
          bool LCDC_BGMapArea = Bit(lcd.regs.LCDC, 3);
          bool LCDC_ObjSize = Bit(lcd.regs.LCDC, 2);
          bool LCDC_ObjEnable = Bit(lcd.regs.LCDC, 1);
          bool LCDC_BGWinEnable = Bit(lcd.regs.LCDC, 0);

          auto tooltip = [](const char *tip) {
            if (ImGui::IsItemHovered()) {
              ImGui::BeginTooltip();
              ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
              ImGui::TextUnformatted(tip);
              ImGui::PopTextWrapPos();
              ImGui::EndTooltip();
            }
          };

          ImGui::BeginGroup();
          ImGui::Checkbox("E", &LCDC_Enable);
          tooltip("LCD Enabled");
          ImGui::SameLine();
          ImGui::Checkbox("WMA", &LCDC_WinMapArea);
          tooltip("Window Map Area");
          ImGui::SameLine();
          ImGui::Checkbox("WE", &LCDC_WindowEnable);
          tooltip("Window Enabled");
          ImGui::SameLine();
          ImGui::Checkbox("TDA", &LCDC_TileDataArea);
          tooltip("Tile Data Area");
          ImGui::SameLine();
          ImGui::Checkbox("BGMA", &LCDC_BGMapArea);
          tooltip("Background Map Area");
          ImGui::SameLine();
          ImGui::Checkbox("OS", &LCDC_ObjSize);
          tooltip("Object Size");
          ImGui::SameLine();
          ImGui::Checkbox("OE", &LCDC_ObjEnable);
          tooltip("Object Enabled");
          ImGui::SameLine();
          ImGui::Checkbox("BGWE", &LCDC_BGWinEnable);
          tooltip("Background/Window Enabled");
          ImGui::EndGroup();

          ImGui::Separator();

          ImGui::BeginTable("linfo", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Window Tilemap Area");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", LCDC_WinMapArea ? "$9C00" : "$9800");

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("BG/Window Tile Data Area");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", LCDC_WinMapArea ? "$8000" : "$8800");

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Object Size");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", LCDC_ObjSize ? "8x16" : "8x8");

          ImGui::EndTable();

          ImGui::Separator();

          ImGui::TextUnformatted(fmt::format("LCD Status {:04b} {:04b}",
                                             (lcd.regs.LCDS & 0xF0) >> 4,
                                             lcd.regs.LCDS & 0xF).c_str());

          bool LCDS_IS_LYCLY = Bit(lcd.regs.LCDS, 6);
          bool LCDS_IS_Mode2_OAM = Bit(lcd.regs.LCDS, 5);
          bool LCDS_IS_Mode1_VBlank = Bit(lcd.regs.LCDS, 4);
          bool LCDS_IS_Mode0_HBlank = Bit(lcd.regs.LCDS, 3);
          bool LCDS_LYCisLY = Bit(lcd.regs.LCDS, 2);

          ImGui::BeginGroup();
          ImGui::Checkbox("LYC", &LCDS_IS_LYCLY);
          tooltip("LYC = LY STAT Interrupt Source");
          ImGui::SameLine();
          ImGui::Checkbox("M2OAM", &LCDS_IS_Mode2_OAM);
          tooltip("Mode 2 OAM Stat Interrupt Source");
          ImGui::SameLine();
          ImGui::Checkbox("M1VB", &LCDS_IS_Mode1_VBlank);
          tooltip("Mode 1 VBlank Stat Interrupt Source");
          ImGui::SameLine();
          ImGui::Checkbox("M0HB", &LCDS_IS_Mode0_HBlank);
          tooltip("Mode 0 HBlank Stat Interrupt Source");
          ImGui::SameLine();
          ImGui::Checkbox("LY=LYC", &LCDS_LYCisLY);
          tooltip("LY Equals LYC");
          ImGui::EndGroup();

          ImGui::Separator();

          ImGui::BeginTable("pinfo", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Mode");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", lcdMode(lcd.LCDS_Mode()));

          ImGui::EndTable();

          ImGui::Separator();
          ImGui::Text("Scroll");

          ImGui::BeginTable("pinfo2", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("X: %d", lcd.regs.SCRX);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("Y: %d", lcd.regs.SCRY);

          ImGui::EndTable();

          ImGui::Separator();
          ImGui::Text("Window");

          ImGui::BeginTable("pinfo3", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("X: %d", lcd.regs.WINX);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("Y: %d", lcd.regs.WINY);

          ImGui::EndTable();

          ImGui::Separator();
          ImGui::Text("Line");

          ImGui::BeginTable("pinfo4", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("LY");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%d", lcd.regs.LY);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("LY Compare");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%d", lcd.regs.LYCP);

          ImGui::EndTable();

          ImGui::Separator();
          ImGui::Text("Palettes");

          ImGui::BeginTable("pinfo5", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("BG Palette");
          ImGui::TableSetColumnIndex(1);

          float sz = ImGui::GetTextLineHeight();
          ImVec2 p = ImGui::GetCursorScreenPos();

          for (auto i = 1; i <= 4; i++) {
            ImGui::GetWindowDrawList()->AddRectFilled(p,
                                                      ImVec2(p.x + sz, p.y + sz),
                                                      IM_COL32(
                                                          lcd.regs.bgColors[i - 1].r,
                                                          lcd.regs.bgColors[i - 1].g,
                                                          lcd.regs.bgColors[i - 1].b,
                                                          lcd.regs.bgColors[i - 1].a
                                                      ));

            p.x += sz + 4.0f;
          }

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("OBJ Palette 0");
          ImGui::TableSetColumnIndex(1);

          sz = ImGui::GetTextLineHeight();
          p = ImGui::GetCursorScreenPos();

          for (auto i = 1; i <= 4; i++) {
            ImGui::GetWindowDrawList()->AddRectFilled(p,
                                                      ImVec2(p.x + sz, p.y + sz),
                                                      IM_COL32(
                                                          lcd.regs.sp1Colors[i - 1].r,
                                                          lcd.regs.sp1Colors[i - 1].g,
                                                          lcd.regs.sp1Colors[i - 1].b,
                                                          lcd.regs.sp1Colors[i - 1].a
                                                      ));

            p.x += sz + 4.0f;
          }

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("OBJ Palette 1");
          ImGui::TableSetColumnIndex(1);
          sz = ImGui::GetTextLineHeight();
          p = ImGui::GetCursorScreenPos();

          for (auto i = 1; i <= 4; i++) {
            ImGui::GetWindowDrawList()->AddRectFilled(p,
                                                      ImVec2(p.x + sz, p.y + sz),
                                                      IM_COL32(
                                                          lcd.regs.sp2Colors[i - 1].r,
                                                          lcd.regs.sp2Colors[i - 1].g,
                                                          lcd.regs.sp2Colors[i - 1].b,
                                                          lcd.regs.sp2Colors[i - 1].a
                                                      ));

            p.x += sz + 4.0f;
          }

          ImGui::EndTable();

          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("State")) {

          ImGui::BeginTable("ppu1", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Line Ticks");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%d", ppu.lineTicks);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Current Frame");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%d", ppu.currentFrame);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Window Line");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%d", ppu.windowLine);

          ImGui::EndTable();

          ImGui::Separator();

          ImGui::BeginTable("ppu2", 2, ImGuiTableFlags_RowBg);
          ImGui::TableSetupColumn("label", ImGuiTableFlags_None);
          ImGui::TableSetupColumn("value", ImGuiTableFlags_None);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Line Sprite Count");
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%d", ppu.lineSpriteCount);


          ImGui::EndTable();

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

      ImGui::End();
    }
  }

} // hijo