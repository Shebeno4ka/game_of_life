#include "game_renderer.hpp"

#include <utility>

GameRenderer::GameRenderer(std::shared_ptr<ThreadSafeGameState> state_ptr,
                           std::shared_ptr<SDL_Renderer> renderer_ptr,
                           GameRunner& game_runner)
    : state_thread_guard_ptr_(std::move(state_ptr)),
      renderer_ptr_(std::move(renderer_ptr)),
      game_runner_(game_runner) {}

void GameRenderer::start() {
  uint64_t size_x;
  uint64_t size_y;
  {
    auto state_guard = state_thread_guard_ptr_->getStateGuard();
    const auto& game_state = state_guard.get();
    size_x = game_state.getField().size();
    size_y = game_state.getField()[0].size();
  }

  bool running = true;
  bool paused = true;

  while (running) {
    handleEvents_(size_x, size_y, running, paused);

    render_();
  }
}

void GameRenderer::handleEvents_(uint64_t size_x, uint64_t size_y,
                                 bool& running, bool& paused) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        game_runner_.stop();
        running = false;
        break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        auto gridX = static_cast<uint32_t>(mouseX / kCellPixels);
        auto gridY = static_cast<uint32_t>(mouseY / kCellPixels);

        if (gridX < size_x && gridY < size_y) {
          auto state_guard = state_thread_guard_ptr_->getStateGuard();
          auto& game_state = state_guard.get();
          game_state.toggleCell(gridX, gridY);
        }
        break;
      }

      case SDL_EVENT_MOUSE_MOTION: {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        auto gridX = static_cast<uint32_t>(mouseX / kCellPixels);
        auto gridY = static_cast<uint32_t>(mouseY / kCellPixels);

        if (gridX < size_x && gridY < size_y) {
          hovered_cell_ = std::make_pair(gridX, gridY);
        } else {
          hovered_cell_.reset(); // курсор вне поля
        }
        break;
      }

      case SDL_EVENT_KEY_DOWN: {
        switch (event.key.key) {
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
          case SDLK_R: {
            auto state_guard = state_thread_guard_ptr_->getStateGuard();
            auto& game_state = state_guard.get();
            game_state.reset();
            break;
          }
          default:
            break;
        }
        break;
      }

      default:
        break;
    }
  }
}

void GameRenderer::render_() {
  SDL_SetRenderDrawColor(renderer_ptr_.get(), kBackgroundColor.r,
                         kBackgroundColor.g, kBackgroundColor.b,
                         kBackgroundColor.a);
  SDL_RenderClear(renderer_ptr_.get());

  drawField_();

  if (hovered_cell_)
    drawHoveredCell_();

  SDL_RenderPresent(renderer_ptr_.get());
}

void GameRenderer::drawField_() {
  auto state_guard = state_thread_guard_ptr_->getStateGuard();
  const auto& field = state_guard.get().getField();

  SDL_SetRenderDrawColor(renderer_ptr_.get(), kCellColor.r, kCellColor.g,
                       kCellColor.b, kCellColor.a);
  for (uint32_t i = 0; i < field.size(); ++i) {
    for (uint32_t j = 0; j < field[i].size(); ++j) {
      if (field[i][j]) {
        const SDL_Rect rect{static_cast<int>(kCellPixels * j),
                            static_cast<int>(i * kCellPixels), kCellPixels,
                            kCellPixels};
        SDL_FRect frect{};
        SDL_RectToFRect(&rect, &frect);
        SDL_RenderFillRect(renderer_ptr_.get(), &frect);
      }
    }
  }
}
void GameRenderer::drawHoveredCell_() {
  SDL_SetRenderDrawColor(renderer_ptr_.get(), kHoveredCellColor.r, kHoveredCellColor.g,
                       kHoveredCellColor.b, kHoveredCellColor.a);
  const auto& [x, y] = *hovered_cell_;
  SDL_Rect rect{static_cast<int>(x * kCellPixels),
                static_cast<int>(y * kCellPixels),
                kCellPixels, kCellPixels};
  SDL_FRect frect{};
  SDL_RectToFRect(&rect, &frect);
  SDL_RenderRect(renderer_ptr_.get(), &frect);
}