#include "sdl_context.hpp"

#include <stdexcept>
#include <string>

SDLContext& SDLContext::GetInstance() {
  // working since c++11
  static SDLContext instance;
  return instance;
}

SDLContext::SDLContext() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
  }
}

SDLContext::~SDLContext() {
  SDL_Quit();
}