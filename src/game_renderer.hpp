#pragma once

#include <SDL_events.h>
#include <SDL_pixels.h>
#include <SDL_render.h>

#include "game_runner.hpp"
#include "thread_safe_game_state.hpp"

using namespace std::chrono_literals;

const int kCellPixels = 10;
const SDL_Color kBackgroundColor = {0, 0, 0, 255};
const SDL_Color kCellColor = {255, 255, 255, 255};

class GameRenderer {
  std::shared_ptr<ThreadSafeGameState> state_ptr_;
  std::shared_ptr<SDL_Renderer> renderer_ptr_;
  GameRunner& game_runner_;

 public:
  GameRenderer(std::shared_ptr<ThreadSafeGameState> state_ptr_,
               std::shared_ptr<SDL_Renderer> renderer_ptr,
               GameRunner& game_runner);

  void start();

 private:
  void handleEvents_(uint32_t& size_x,
                     uint32_t& size_y,
                     bool& running,
                     bool& paused);
  void render_();
  void drawField_();
};