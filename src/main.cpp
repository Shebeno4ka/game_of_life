#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_render.h>

#include <iostream>
#include <memory>

#include "console.hpp"
#include "game_renderer.hpp"
#include "game_runner.hpp"
#include "game_state.hpp"
#include "sdl_context.hpp"

using sdl_window_ptr = std::shared_ptr<SDL_Window>;

int startGame(Options& opts) {
  SDLContext::GetInstance();

  uint32_t window_side_size = opts.size_y * kCellPixels;

  auto sdl_window = SDL_CreateWindow(
      "Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      static_cast<int>(window_side_size), static_cast<int>(window_side_size),
      SDL_WINDOW_SHOWN);
  if (!sdl_window) {
    std::cerr << "Window creation error: " << SDL_GetError() << std::endl;
    return 1;
  }
  std::shared_ptr<SDL_Window> window_ptr(sdl_window, &SDL_DestroyWindow);

  SDL_Renderer* renderer =
      SDL_CreateRenderer(window_ptr.get(), -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "Renderer creation error: " << SDL_GetError() << std::endl;
    return 1;
  }
  std::shared_ptr<SDL_Renderer> renderer_ptr(renderer, &SDL_DestroyRenderer);

  std::vector<std::vector<bool>> field(opts.size_y,
                                       std::vector<bool>(opts.size_x, false));

  auto state = std::make_shared<ThreadSafeGameState>(
      std::make_shared<GameState>(&field));

  GameRunner game_runner(state, opts.updates_per_second);
  GameRenderer game_renderer(state, renderer_ptr, game_runner);
  game_runner.start();
  game_renderer.start();

  return 0;
}

int main(int argc, char* argv[]) {
  std::optional<Options> opts;

  try {
    opts = ParseCommandLineOptions(argc, argv);
  } catch (const InvalidArgumentException& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cerr << "Type \"GameOfLife -h\" for help" << std::endl;
    return 1;
  }

  if (!opts) {
    std::cout << kHelpText;
    return 0;
  }

  return startGame(*opts);
}
