#include "Controller.h"

#include "system/Gameboy.h"
#include "cpu/Interrupts.h"

namespace hijo {
  Controller::Controller() {
    Init();
  }

  Controller::~Controller() {
    EventManager::Get().DetachAll(this);
  }

  void Controller::Init() {
    EventManager::Get().Attach<
        Events::KeyDown,
        &Controller::HandleKeyDown
    >(this);

    EventManager::Get().Attach<
        Events::KeyUp,
        &Controller::HandleKeyUp
    >(this);
  }

  bool Controller::ButtonSelected() {
    return m_ButtonSelected;
  }

  bool Controller::DirectionSelected() {
    return m_DirectionSelected;
  }

  void Controller::SetSelected(uint8_t value) {
    m_ButtonSelected = value & 0x20;
    m_DirectionSelected = value & 0x10;
  }

  uint8_t Controller::Output() {
    uint8_t output = 0xCF;

    if (!m_ButtonSelected) {
      if (m_Buttons.start) {
        output &= ~(1 << 3);
      }

      if (m_Buttons.select) {
        output &= ~(1 << 2);

      }

      if (m_Buttons.a) {
        output &= ~(1 << 0);

      }

      if (m_Buttons.b) {
        output &= ~(1 << 1);
      }
    }

    if (!m_DirectionSelected) {
      if (m_Buttons.left) {
        output &= ~(1 << 1);
      }

      if (m_Buttons.right) {
        output &= ~(1 << 0);
      }

      if (m_Buttons.up) {
        output &= ~(1 << 2);
      }

      if (m_Buttons.down) {
        output &= ~(1 << 3);
      }
    }

    return output;
  }

  void Controller::HandleKeyDown(const Events::KeyDown &event) {
    bool pressed = false;

    switch (event.key) {
      case KEY_W:
        m_Buttons.up = true;
        pressed = true;
        break;

      case KEY_A:
        m_Buttons.left = true;
        pressed = true;
        break;

      case KEY_S:
        m_Buttons.down = true;
        pressed = true;
        break;

      case KEY_D:
        m_Buttons.right = true;
        pressed = true;
        break;

      case KEY_ENTER:
        m_Buttons.start = true;
        pressed = true;
        break;

      case KEY_TAB:
        m_Buttons.select = true;
        pressed = true;
        break;

      case KEY_PERIOD:
        m_Buttons.a = true;
        pressed = true;
        break;

      case KEY_COMMA:
        m_Buttons.b = true;
        pressed = true;
        break;
    }

    if (pressed) {
      auto &bus = Gameboy::Get();

      Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::Joypad);
    }
  }

  void Controller::HandleKeyUp(const Events::KeyUp &event) {
    switch (event.key) {
      case KEY_W:
        m_Buttons.up = false;
        break;

      case KEY_A:
        m_Buttons.left = false;
        break;

      case KEY_S:
        m_Buttons.down = false;
        break;

      case KEY_D:
        m_Buttons.right = false;
        break;

      case KEY_ENTER:
        m_Buttons.start = false;
        break;

      case KEY_TAB:
        m_Buttons.select = false;
        break;

      case KEY_PERIOD:
        m_Buttons.a = false;
        break;

      case KEY_COMMA:
        m_Buttons.b = false;
        break;
    }
  }
} // hijo