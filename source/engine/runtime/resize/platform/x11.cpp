#include "../handler.hpp"

#if !BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK || !BAPHOMETS_QUEST_PLATFORM_X11
#error "resize/x11.cpp should only be compiled for native X11 aspect locking"
#endif

#include <SDL_syswm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace Engine::ResizeHandler {

void lockAspectRatio(SDL_Window *window, SDL_Renderer *renderer) {
    int logicalWidth = 0;
    int logicalHeight = 0;
    SDL_RenderGetLogicalSize(renderer, &logicalWidth, &logicalHeight);

    if(logicalWidth <= 0 || logicalHeight <= 0) {
        return;
    }

    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);

    if(SDL_GetWindowWMInfo(window, &windowInfo) != SDL_TRUE
        || windowInfo.subsystem != SDL_SYSWM_X11
        || windowInfo.info.x11.display == nullptr) {
        return;
    }

    XSizeHints hints{};
    hints.flags = PAspect;
    hints.min_aspect.x = logicalWidth;
    hints.min_aspect.y = logicalHeight;
    hints.max_aspect.x = logicalWidth;
    hints.max_aspect.y = logicalHeight;

    XSetWMNormalHints(
        windowInfo.info.x11.display,
        windowInfo.info.x11.window,
        &hints
    );
    XFlush(windowInfo.info.x11.display);
}

} // namespace Engine::ResizeHandler
