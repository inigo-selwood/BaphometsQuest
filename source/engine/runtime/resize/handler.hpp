#pragma once

#include <SDL.h>

namespace Engine::ResizeHandler {

/** Keep a resizable window aligned to the renderer logical aspect ratio */
void handle(
    SDL_Window *window,
    SDL_Renderer *renderer,
    const SDL_Event &event
);

/** Ask the native window to preserve the renderer logical aspect ratio */
void lockAspectRatio(SDL_Window *window, SDL_Renderer *renderer);

} // namespace Engine::ResizeHandler
