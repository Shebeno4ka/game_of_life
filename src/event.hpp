#pragma once

#include <cstdint>

struct GameEvent {
  enum Type {
    None,
    CellToggle,
    GamePauseToggle,
  };

  Type type_;

  struct CellToggle {
    uint32_t x;
    uint32_t y;
  };

  struct GamePauseToggle {};

  union {
    struct CellToggle cell_toggle_;
    struct GamePauseToggle game_pause_toggle_;
  };
};