#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

#include <memory>
#include <optional>
#include <utility>
#include <chrono>

#include "core/GameSimulator.h"
#include "server/GameServer.h"
#include "server/GameServerConcept.h"
#include "thread_safe_game_state.hpp"

using namespace std::chrono_literals;

constexpr uint16_t kCellPixels = 10;
constexpr SDL_Color kBackgroundColor = {0, 0, 0, 255};
constexpr SDL_Color kCellColor = {255, 255, 255, 255};
constexpr SDL_Color kHoveredCellColor = {255, 0, 0, 128};

/**
 * \class GameRenderer
 * \brief Handles rendering of the game field and user interactions.
 */
template <LifeGame::GameServerConcept GameServer>
class GameSDLRenderer {
    std::shared_ptr<GameServer> game_server_;
    std::optional<std::pair<uint32_t, uint32_t>> hovered_cell_coords_;

   public:
    /**
     * \brief Constructs a GameRenderer instance.
     * \param state_ptr Shared pointer to thread-safe game state.
     * \param renderer_ptr Shared pointer to SDL renderer.
     * \param game_runner Unique pointer to started game runner instance.
     */
    GameSDLRenderer(std::shared_ptr<GameServer> game_server) : game_server_(std::move(game_server)) {
        game_server_.get().setMessageCallback(render_);
    }

    /**
     * \brief Starts the rendering process.
     */
    void start() {
        uint64_t size_x;
        uint64_t size_y;
        {
            auto state_guard = state_thread_guard_ptr_->getStateGuard();
            auto& game_state = state_guard.get();
            size_x = game_state.getField()[0].size();
            size_y = game_state.getField().size();
        }

        bool running = true;
        bool paused = true;

        while (running) {
            handleEvents_(size_x, size_y, running, paused);
            render_();
        }
    }

   private:
    /**
     * \brief Handles user input events and updates the game state.
     * \param size_x Width of the game field in cells.
     * \param size_y Height of the game field in cells.
     * \param running Reference to the running state of the game loop.
     * \param paused Reference to the paused state of the game loop.
     */
    void handleEvents_(uint64_t size_x, uint64_t size_y, bool& running, bool& paused) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    game_server_->stop();
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

                case SDL_EVENT_KEY_DOWN: {
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            game_server_->stop();
                            running = false;
                            break;
                        case SDLK_SPACE:
                            if (paused) {
                                game_server_->resume();
                            } else {
                                game_server_->pause();
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

                case SDL_EVENT_MOUSE_MOTION: {
                    float mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    auto gridX = static_cast<uint32_t>(mouseX / kCellPixels);
                    auto gridY = static_cast<uint32_t>(mouseY / kCellPixels);

                    if (gridX < size_x && gridY < size_y) {
                        hovered_cell_coords_ = std::make_pair(gridX, gridY);
                    } else {
                        hovered_cell_coords_.reset(); // курсор вне поля
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    /**
     * \brief Renders the game field and other visual elements.
     */
    void render_() {
        SDL_SetRenderDrawColor(renderer_ptr_.get(), kBackgroundColor.r, kBackgroundColor.g, kBackgroundColor.b,
                               kBackgroundColor.a);
        SDL_RenderClear(renderer_ptr_.get());

        drawField_();

        if (hovered_cell_coords_)
            drawHoveredCell_();

        SDL_RenderPresent(renderer_ptr_.get());
    }

    /**
     * \brief Draws the game field based on the current game state.
     */
    void drawField_() {
        auto state_guard = state_thread_guard_ptr_->getStateGuard();
        const auto& field = state_guard.get().getField();

        SDL_SetRenderDrawColor(renderer_ptr_.get(), kCellColor.r, kCellColor.g, kCellColor.b, kCellColor.a);
        for (uint32_t i = 0; i < field.size(); ++i) {
            for (uint32_t j = 0; j < field[i].size(); ++j) {
                if (field[i][j]) {
                    const SDL_Rect rect{static_cast<int>(kCellPixels * j), static_cast<int>(i * kCellPixels),
                                        kCellPixels, kCellPixels};
                    SDL_FRect frect{};
                    SDL_RectToFRect(&rect, &frect);
                    SDL_RenderFillRect(renderer_ptr_.get(), &frect);
                }
            }
        }
    }

    /**
     * \brief Highlights the currently hovered cell.
     */
    void drawHoveredCell_() {
        SDL_SetRenderDrawColor(renderer_ptr_.get(), kHoveredCellColor.r, kHoveredCellColor.g, kHoveredCellColor.b,
                               kHoveredCellColor.a);
        const auto& [x, y] = *hovered_cell_coords_;
        SDL_Rect rect{static_cast<int>(x * kCellPixels), static_cast<int>(y * kCellPixels), kCellPixels, kCellPixels};
        SDL_FRect frect{};
        SDL_RectToFRect(&rect, &frect);
        SDL_RenderRect(renderer_ptr_.get(), &frect);
    }
};