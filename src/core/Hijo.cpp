
#include "config.h"
#include "input/Input.h"
#include "events/EventManager.h"
#include "events/Events.h"

#include "Hijo.h"

#include "layers/Emu.h"
#include "layers/UI.h"

#include "system/Gameboy.h"

#include <SDL.h>

namespace hijo {
  void Hijo::Run() {
    if (!m_GameLayers)
      return;

    m_GameLayers->PushState(CreateRef<Emu>());
    m_GameLayers->PushOverlay(CreateRef<UI>());

    double lastTime = GetTime();

    m_PreviousMousePosition = GetMousePosition();

    do {
      m_CurrentTime = GetTime();
      m_Timestep = m_CurrentTime - lastTime;

      m_WorldCoords = GetScreenToWorld2D(m_MousePosition, m_Camera);

      Input::Manager::Get().Poll();

      // Dispatch queued events
      EventManager::Dispatcher().update();

      for (const auto &layer: *m_GameLayers) {
        layer->Update(m_Timestep);
      }

      auto displayTile = [](uint16_t startLocation, uint16_t tileNum, int x, int y) {
        auto &gb = Gameboy::Get();

        std::vector<Color> tileColors{
            WHITE,
            {0xAA, 0xAA, 0xAA, 0xFF},
            {0x55, 0x55, 0x55, 0xFF},
            BLACK
        };

        for (int tileY = 0; tileY < 16; tileY += 2) {
          uint8_t b1 = gb.cpuRead(startLocation + (tileNum * 16) + tileY);
          uint8_t b2 = gb.cpuRead(startLocation + (tileNum * 16) + tileY + 1);

          for (int bit = 7; bit >= 0; bit--) {
            uint8_t high = (!!(b1 & (1 << bit))) << 1;
            uint8_t low = !!(b2 & (1 << bit));
            uint8_t color = high | low;

            auto rx = x + ((7 - bit) * 4);
            auto ry = y + (tileY / 2 * 4);
            auto w = 4;
            auto h = 4;

            DrawRectangle(rx, ry, w, h, tileColors[color]);
          }
        }
      };


      BeginTextureMode(m_TileTexture);
      ClearBackground(m_DefaultBackground);
      {
        uint16_t addr = 0x8000;
        int xDraw = 0;
        int yDraw = 0;
        int tileNum = 0;

        for (int y = 0; y < 24; y++) {
          for (int x = 0; x < 16; x++) {
            displayTile(addr, tileNum, xDraw + (x * 4), yDraw + (y * 4));
            xDraw += (8 * 4);
            tileNum++;
          }

          yDraw += (8 * 4);
          xDraw = 0;
        }
      }
      EndTextureMode();

      BeginTextureMode(m_PackedTileTexture);
      ClearBackground(m_DefaultBackground);
      {
        uint16_t addr = 0x8000;
        int xDraw = 0;
        int yDraw = 0;
        int tileNum = 0;

        for (int y = 0; y < 24; y++) {
          for (int x = 0; x < 16; x++) {
            displayTile(addr, tileNum, xDraw, yDraw);
            xDraw += (8 * 4);
            tileNum++;
          }

          yDraw += (8 * 4);
          xDraw = 0;
        }
      }
      EndTextureMode();

      BeginDrawing();

      for (const auto &layer: *m_GameLayers) {
        layer->BeginFrame();
      }

      ClearBackground(m_DefaultBackground);


      for (auto layer: *m_GameLayers) {
        if (!layer->RenderTarget())
          layer->Render();
      }

      for (const auto &layer: *m_GameLayers) {
        layer->EndFrame();
      }

      EndDrawing();

      EventManager::Dispatcher().trigger(Events::HandleAudio{});

      lastTime = m_CurrentTime;
    } while (m_Running && !WindowShouldClose());
  }

  void Hijo::Initialize() {
    SetTraceLogCallback(RaylibTraceCallback);

    m_Title = fmt::format("Hijo v{}", HIJO_VERSION);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title.c_str());

    SetWindowMinSize(100, 100);

    SetExitKey(0);

    SetTargetFPS(62);

    while (!IsWindowReady());
    m_Camera.target = {0, 0};
    m_Camera.offset = {0, 0};
    m_Camera.rotation = 0.0f;
    m_Camera.zoom = 1.0f;

    m_GameLayers = new GameLayerStack();

    float aspect = 10.0f / 9.0f;
    m_ScreenWidth = static_cast<int32_t>(m_ScreenHeight * aspect);

    m_RenderTexture = LoadRenderTexture(m_ScreenWidth, m_ScreenHeight);
    SetTextureFilter(m_RenderTexture.texture, TEXTURE_FILTER_POINT);

    m_TileTexture = LoadRenderTexture(36 * 16, 36 * 24);
    SetTextureFilter(m_TileTexture.texture, TEXTURE_FILTER_POINT);

    m_PackedTileTexture = LoadRenderTexture(32 * 16, 32 * 24);
    SetTextureFilter(m_PackedTileTexture.texture, TEXTURE_FILTER_POINT);

    SDL_Init(SDL_INIT_AUDIO);

    EventManager::Get().Attach<
        Events::ViewportResized,
        &Hijo::HandleViewportResized
    >(this);

    EventManager::Get().Attach<
        Events::UIMouseMove,
        &Hijo::HandleMouseMove
    >(this);

    EventManager::Get().Attach<
        Events::WantQuit,
        &Hijo::HandleWantQuit
    >(this);

    EventManager::Get().Attach<
        Events::VBlank,
        &Hijo::HandleVBlank
    >(this);
  }

  Hijo::~Hijo() {
    EventManager::Get().DetachAll(this);

    delete m_GameLayers;

    UnloadRenderTexture(m_RenderTexture);
    UnloadRenderTexture(m_TileTexture);
    UnloadRenderTexture(m_PackedTileTexture);
    SDL_Quit();
    CloseWindow();
  }

  void Hijo::RaylibTraceCallback(int logLevel, const char *text, va_list args) {
    char buffer[4096];

    vsnprintf(buffer, 4096, text, args);
    buffer[4095] = '\0';

    auto log = spdlog::get("raylib");

    switch (logLevel) {
      case LOG_TRACE:
        log->trace("{}", buffer);
        break;

      case LOG_DEBUG:
        log->debug("{}", buffer);
        break;

      case LOG_INFO:
        log->info("{}", buffer);
        break;

      case LOG_WARNING:
        log->warn("{}", buffer);
        break;

      case LOG_ERROR:
        log->error("{}", buffer);
        break;

      case LOG_FATAL:
        log->critical("{}", buffer);
        break;

      default:
        break;
    }
  }

  void Hijo::HandleViewportResized(const Events::ViewportResized &event) {
    UnloadRenderTexture(m_RenderTexture);

    m_ViewportSize.x = event.x;
    m_ViewportSize.y = event.y;

    float aspect = 10.0f / 9.0f;

    m_ScreenWidth = static_cast<int32_t>(event.y * aspect);
    m_ScreenHeight = event.y;

    m_RenderTexture = LoadRenderTexture(m_ScreenWidth, m_ScreenHeight);
    SetTextureFilter(m_RenderTexture.texture, TEXTURE_FILTER_POINT);
  }

  void Hijo::HandleMouseMove(const Events::UIMouseMove &event) {
    m_PreviousMousePosition = m_MousePosition;

    m_MousePosition = event.position;
  }

  void Hijo::HandleWantQuit(const Events::WantQuit &) {
    m_Running = false;
  }

  void Hijo::HandleVBlank(const Events::VBlank &) {
    BeginTextureMode(m_RenderTexture);
    ClearBackground(m_DefaultBackground);
    for (auto layer: *m_GameLayers) {
      if (layer->RenderTarget()) {
        layer->Render();
      }
    }
    EndTextureMode();
  }

} // hijo