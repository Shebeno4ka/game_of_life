#pragma once

#include <SDL.h>
#include <SDL_stdinc.h>

/**
 * @class SDLContext
 * @brief Singleton class that handles global SDL initialization and cleanup
 */
class SDLContext {
 public:
  static SDLContext& GetInstance();
  SDLContext(const SDLContext&) = delete;
  SDLContext& operator=(const SDLContext&) = delete;
  ~SDLContext();

 private:
  SDLContext();
};