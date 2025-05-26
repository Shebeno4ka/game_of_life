#include "game_renderer.hpp"

GameRenderer::GameRenderer(std::shared_ptr<ThreadSafeGameState> state_ptr,
                           std::shared_ptr<SDL_Renderer> renderer_ptr,
                           GameRunner& game_runner)
    : state_ptr_(state_ptr),
      renderer_ptr_(renderer_ptr),
      game_runner_(game_runner) {}

void GameRenderer::start() {
  uint64_t size_x;
  uint64_t size_y;
  {
    auto field = state_ptr_->getField().getField();
    size_x = field.size();
    size_y = field[0].size();
  }

  bool running = true;
  bool paused = true;

  while (running) {
    handleEvents_(size_x, size_y, running, paused);

    render_();

    std::this_thread::sleep_for(10ms);
  }
}

void GameRenderer::handleEvents_(uint64_t size_x,
                                 uint64_t size_y,
                                 bool& running,
                                 bool& paused) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      game_runner_.stop();
      running = false;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      int mouseX, mouseY;
      SDL_GetMouseState(&mouseX, &mouseY);

      // Convert mouse position to grid coordinates
      uint32_t gridX = static_cast<uint32_t>(mouseX / kCellPixels);
      uint32_t gridY = static_cast<uint32_t>(mouseY / kCellPixels);

      // Toggle the cell state
      if (gridX >= 0 && gridX < size_x && gridY >= 0 && gridY < size_y) {
        state_ptr_->toggleCell(gridX, gridY);
      }
    } else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          game_runner_.stop();
          running = false;
          break;
        case SDLK_SPACE:
          if (paused) {
            game_runner_.resume();
          } else {
            game_runner_.pause();
          }
          paused = !paused;
          break;
        case SDLK_r:
          state_ptr_->reset();  // Reset the game state
          break;
        default:
          break;
      }
    }
  }
}

void GameRenderer::render_() {
  SDL_SetRenderDrawColor(renderer_ptr_.get(), kBackgroundColor.r,
                         kBackgroundColor.g, kBackgroundColor.b,
                         kBackgroundColor.a);
  SDL_RenderClear(renderer_ptr_.get());

  drawField_();

  SDL_RenderPresent(renderer_ptr_.get());
}

void GameRenderer::drawField_() {
  const auto& field = state_ptr_->getField();

  for (uint32_t i = 0; i < field.getField().size(); ++i) {
    for (uint32_t j = 0; j < field.getField()[i].size(); ++j) {
      if (field.getField()[i][j]) {
        SDL_SetRenderDrawColor(renderer_ptr_.get(), kCellColor.r, kCellColor.g,
                               kCellColor.b, kCellColor.a);
        SDL_Rect rect{static_cast<int>(j * kCellPixels),
                      static_cast<int>(i * kCellPixels), kCellPixels,
                      kCellPixels};
        SDL_RenderFillRect(renderer_ptr_.get(), &rect);
      }
    }
  }
}