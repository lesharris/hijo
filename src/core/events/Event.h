#pragma once

namespace hijo::Events {
  struct Event {
    Event() : handled(false) {};

    ~Event() = default;

    bool handled;
  };
}