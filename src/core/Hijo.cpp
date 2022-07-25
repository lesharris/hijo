
#include "config.h"
#include "input/Input.h"
#include "events/EventManager.h"
#include "events/Events.h"

#include "Hijo.h"
#include "layers/Sandbox.h"

namespace hijo {
  void Hijo::Run() {
    m_GameLayers->PushState(CreateRef<Sandbox>());

    double lastTime = GetTime();

    m_PreviousMousePosition = GetMousePosition();

    do {
      m_CurrentTime = GetTime();
      m_Timestep = m_CurrentTime - lastTime;

      if (IsWindowResized()) {
        m_ScreenWidth = GetScreenWidth();
        m_ScreenHeight = GetScreenHeight();
        SetWindowSize(m_ScreenWidth, m_ScreenHeight);
        EventManager::Dispatcher().enqueue<Events::WindowResized>({m_ScreenWidth, m_ScreenHeight});
      }

      auto mousePosition = GetMousePosition();

      if (m_PreviousMousePosition.x != mousePosition.x ||
          m_PreviousMousePosition.y != mousePosition.y) {
        m_PreviousMousePosition = m_MousePosition;
        m_MousePosition = mousePosition;
      }

      m_WorldCoords = GetScreenToWorld2D(m_MousePosition, m_Camera);

      Input::Manager::Get().Poll();

      for (const auto &layer: *m_GameLayers) {
        layer->Update(m_Timestep);
      }

      for (const auto &layer: *m_GameLayers) {
        layer->RenderTexture();
      }

      BeginDrawing();

      ClearBackground(m_DefaultBackground);

      for (const auto &layer: *m_GameLayers) {
        layer->BeginFrame();
      }

      for (const auto &layer: *m_GameLayers) {
        layer->Render();
      }

      for (const auto &layer: *m_GameLayers) {
        layer->EndFrame();
      }

      EndDrawing();

      m_PreviousMousePosition = m_MousePosition;
      lastTime = m_CurrentTime;
    } while (!WindowShouldClose());
  }

  void Hijo::Initialize() {
    SetTraceLogCallback(RaylibTraceCallback);

    m_Title = fmt::format("Hijo v{}", HIJO_VERSION);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title.c_str());

    SetWindowMinSize(100, 100);

    SetExitKey(0);

    SetTargetFPS(60);

    while (!IsWindowReady());

    InitAudioDevice();

    m_Camera.target = {0, 0};
    m_Camera.offset = {0, 0};
    m_Camera.rotation = 0.0f;
    m_Camera.zoom = 1.0f;

    m_GameLayers = new GameLayerStack();


  }

  Hijo::~Hijo() {
    EventManager::Get().DetachAll(this);

    delete m_GameLayers;

    CloseAudioDevice();
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

} // hijo