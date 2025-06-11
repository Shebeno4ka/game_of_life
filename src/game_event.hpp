/*
* std::visit(EventVisitor{}, event);
*/

#pragma once

#include <cstdint>
#include <variant>


struct AddCellEvent {
  uint32 x;
  iunt32 y;
};

struct PauseEvent {};

struct UnPauseEvent {};

struct EventVisitor {
  void operator()(const AddCellEvent& e) {
    throw UnimplementedException("Unimplemented1");
  }

  void operator()(const PauseEvent& e) {
    throw UnimplementedException("Unimplemented2");
  }

  void operator()(const UnPauseEvent& e) {
    throw UnimplementedException("Unimplemented3");
  }
};

using GameEvent = std::variant<AddCellEvent, PauseEvent, UnPauseEvent>;