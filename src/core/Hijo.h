#pragma once

#include "raylib.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "common/common.h"
#include "core/layers/GameLayerStack.h"
#include "core/events/EventManager.h"

namespace hijo {

  class Hijo {
  public:
    static Hijo &Get() {
      static Hijo instance;

      return instance;
    }

    void Run();

    Vector2 MousePosition() { return m_MousePosition; }

    Vector2 PreviousMousePosition() { return m_PreviousMousePosition; }

    Vector2 WorldCoords() { return m_WorldCoords; }

    RenderTexture &GetRenderTexture() { return m_RenderTexture; }

  private:
    Hijo() {
      m_ConsoleSink = spdlog::stdout_color_mt("console");
      m_RaylibSink = spdlog::stdout_color_mt("raylib");

      Initialize();
    };

    ~Hijo();

    void Initialize();

  private:
    static void RaylibTraceCallback(int logLevel, const char *text, va_list args);

  private:
    void HandleViewportResized(const Events::ViewportResized &event);

    void HandleMouseMove(const Events::UIMouseMove &event);

  private:
    GameLayerStack *m_GameLayers = nullptr;

    Ref<spdlog::logger> m_ConsoleSink;
    Ref<spdlog::logger> m_RaylibSink;

    std::string m_Title;

    Color m_DefaultBackground{30, 30, 30, 255};

    int32_t m_ScreenWidth = 800;
    int32_t m_ScreenHeight = 600;

    Vector2 m_ViewportSize{800, 600};

    double m_CurrentTime = 0;
    double m_Timestep = 0;

    Vector2 m_MousePosition{};
    Vector2 m_PreviousMousePosition{};
    Vector2 m_WorldCoords{};

    Camera2D m_Camera{};
    RenderTexture m_RenderTexture;
  };

} // hijo

