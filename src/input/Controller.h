#pragma once

#include <cstdint>

#include "core/events/EventManager.h"

namespace hijo {

  class Controller {
  public:
    struct ButtonsState {
      bool start;
      bool select;
      bool a;
      bool b;
      bool up;
      bool down;
      bool left;
      bool right;
    };
  public:
    Controller();

    ~Controller();

    void Init();

    bool ButtonSelected();

    bool DirectionSelected();

    void SetSelected(uint8_t value);

    uint8_t Output();

    ButtonsState &Buttons() {
      return m_Buttons;
    }

  private:
    void HandleKeyDown(const Events::KeyDown &event);

    void HandleKeyUp(const Events::KeyUp &event);

  private:
    ButtonsState m_Buttons;
    bool m_ButtonSelected;
    bool m_DirectionSelected;
  };

} // hijo
