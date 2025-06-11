/*
 * std::visit(EventVisitor{}, event);
 */

#pragma once

#include <cstdint>
#include <variant>

struct AddCellEvent {
  uint32_t x;
  uint32_t y;
};

struct PauseEvent {};

struct UnPauseEvent {};

using GameEvent = std::variant<AddCellEvent, PauseEvent, UnPauseEvent>;