#include "handler.hpp"

#include <cmath>

namespace Engine::ResizeHandler {

namespace {

bool isResizeEvent(SDL_Window *window, const SDL_Event &event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.windowID == SDL_GetWindowID(window)
        && (event.window.event == SDL_WINDOWEVENT_RESIZED
            || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED);
}

} // namespace

void handle(
    SDL_Window *window,
    SDL_Renderer *renderer,
    const SDL_Event &event
) {
    if(!isResizeEvent(window, event)) {
        return;
    }

    int logicalWidth = 0;
    int logicalHeight = 0;
    SDL_RenderGetLogicalSize(renderer, &logicalWidth, &logicalHeight);

    if(logicalWidth <= 0 || logicalHeight <= 0) {
        return;
    }

    const int resizedWidth = event.window.data1;
    const int resizedHeight = event.window.data2;

    const float logicalAspect =
        static_cast<float>(logicalWidth) / static_cast<float>(logicalHeight);
    const float resizedAspect =
        static_cast<float>(resizedWidth) / static_cast<float>(resizedHeight);

    int targetWidth = resizedWidth;
    int targetHeight = resizedHeight;

    if(resizedAspect > logicalAspect) {
        targetWidth =
            static_cast<int>(std::round(resizedHeight * logicalAspect));
    } else {
        targetHeight =
            static_cast<int>(std::round(resizedWidth / logicalAspect));
    }

    if(targetWidth != resizedWidth || targetHeight != resizedHeight) {
        SDL_SetWindowSize(window, targetWidth, targetHeight);
    }
}

} // namespace Engine::ResizeHandler
