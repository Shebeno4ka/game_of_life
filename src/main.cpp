#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include <iostream>
#include <memory>

#include "console.hpp"
#include "game_renderer.hpp"
#include "game_runner.hpp"
#include "game_state.hpp"

using sdl_window_ptr = std::shared_ptr<SDL_Window>;

int startGame(Options &opts) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  uint32_t window_side_size = opts.size_y * kCellPixels;

  auto sdl_window = SDL_CreateWindow(
      "Game of Life",
      static_cast<int>(window_side_size), static_cast<int>(window_side_size),
      SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!sdl_window) {
    std::cerr << "Window creation error: " << SDL_GetError() << std::endl;
    return 1;
  }
  std::shared_ptr<SDL_Window> window_ptr(sdl_window, &SDL_DestroyWindow);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window_ptr.get(), nullptr);
  if (!renderer) {
    std::cerr << "Renderer creation error: " << SDL_GetError() << std::endl;
    return 1;
  }
  std::shared_ptr<SDL_Renderer> renderer_ptr(renderer, &SDL_DestroyRenderer);

  std::vector field(opts.size_y, std::vector<bool>(opts.size_x, false));

  auto state = std::make_shared<ThreadSafeGameState>(
      std::make_shared<GameState>(&field));

  auto game_runner = std::make_unique<GameRunner>(state, opts.updates_per_second);
  game_runner->start();

  GameRenderer game_renderer(state, renderer_ptr, std::move(game_runner));
  game_renderer.start();

  SDL_Quit();
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
