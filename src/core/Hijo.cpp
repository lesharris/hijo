
#include "config.h"
#include "input/Input.h"
#include "events/EventManager.h"
#include "events/Events.h"

#include "Hijo.h"

#include "layers/Sandbox.h"
#include "layers/UI.h"

namespace hijo {
  void Hijo::Run() {
    if (!m_GameLayers)
      return;

    m_GameLayers->PushState(CreateRef<Sandbox>());
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

      BeginTextureMode(m_RenderTexture);
      ClearBackground(m_DefaultBackground);
      for (auto layer: *m_GameLayers) {
        if (layer->RenderTarget()) {
          layer->Render();
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

    SetTargetFPS(60);

    while (!IsWindowReady());

    InitAudioDevice();

    m_Camera.target = {0, 0};
    m_Camera.offset = {0, 0};
    m_Camera.rotation = 0.0f;
    m_Camera.zoom = 1.0f;

    m_GameLayers = new GameLayerStack();

    float aspect = 10.0f / 9.0f;
    m_ScreenWidth = static_cast<int32_t>(m_ScreenHeight * aspect);

    m_RenderTexture = LoadRenderTexture(m_ScreenWidth, m_ScreenHeight);
    SetTextureFilter(m_RenderTexture.texture, TEXTURE_FILTER_POINT);

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
  }

  Hijo::~Hijo() {
    EventManager::Get().DetachAll(this);

    delete m_GameLayers;

    UnloadRenderTexture(m_RenderTexture);

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

  void Hijo::HandleViewportResized(const Events::ViewportResized &event) {
    UnloadRenderTexture(m_RenderTexture);

    m_ViewportSize.x = event.x;
    m_ViewportSize.y = event.y;

    float aspect = 10.0f / 9.0f;

    m_ScreenWidth = static_cast<int32_t>(event.y * aspect);
    m_ScreenHeight = event.y;

    m_RenderTexture = LoadRenderTexture(m_ScreenWidth, m_ScreenHeight);
    SetTextureFilter(m_RenderTexture.texture, TEXTURE_FILTER_POINT);

    spdlog::get("console")->info("{} {}", m_ScreenWidth, m_ScreenHeight);
  }

  void Hijo::HandleMouseMove(const Events::UIMouseMove &event) {
    m_PreviousMousePosition = m_MousePosition;

    m_MousePosition = event.position;
  }

  void Hijo::HandleWantQuit(const Events::WantQuit &) {
    m_Running = false;
  }

} // hijo