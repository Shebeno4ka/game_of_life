#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include <iostream>
#include <memory>

#include "console.hpp"
#include "game_runner.hpp"
#include "game_sdl_renderer.hpp"
#include "game_state.hpp"
#include "thread_safe_game_state.hpp"

using sdl_window_ptr = std::shared_ptr<SDL_Window>;

int startGame(Options& opts) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  auto sdl_window = SDL_CreateWindow("Game of Life", opts.size_x * kCellPixels,
                                     opts.size_y * kCellPixels,
                                     SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!sdl_window) {
    std::cerr << "Window creation error: " << SDL_GetError() << std::endl;
    return 1;
  }
  std::shared_ptr<SDL_Window> window_ptr(sdl_window, &SDL_DestroyWindow);

  SDL_Renderer* renderer = SDL_CreateRenderer(window_ptr.get(), nullptr);
  if (!renderer) {
    std::cerr << "Renderer creation error: " << SDL_GetError() << std::endl;
    return 1;
  }
  std::shared_ptr<SDL_Renderer> renderer_ptr(renderer, &SDL_DestroyRenderer);

  SDL_SetRenderVSync(renderer_ptr.get(), 1);

  std::vector field(static_cast<size_t>(opts.size_y),
                    std::vector(static_cast<size_t>(opts.size_x), false));

  GameState&& game_state = GameState(std::move(field));
  ThreadSafeGameState&& thread_safe_game_state =
      ThreadSafeGameState(std::move(game_state));

  auto game_runner = std::make_unique<GameRunner>(
      std::move(thread_safe_game_state), opts.updates_per_second);
  game_runner->start();

  // GameSDLRenderer game_renderer(game_state, renderer_ptr,
  //                               std::move(game_runner));
  // game_renderer.start();

  SDL_Quit();
  return 0;
}

int main(int argc, char* argv[]) {
  Options opts_t{50, 100, 10};
  return startGame(opts_t);
  std::optional<Options> opts;

  try {
    opts = ParseCommandLineOptions(argc, argv);
  } catch (const invalid_argument_exception& e) {
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
