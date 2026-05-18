#include "../handler.hpp"

#if !BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK || !BAPHOMETS_QUEST_PLATFORM_MACOS
#error                                                                        \
    "resize/macos.cpp should only be compiled for native macOS aspect locking"
#endif

#include <SDL_syswm.h>
#include <objc/message.h>
#include <objc/runtime.h>

namespace Engine::ResizeHandler {

namespace {

struct CocoaSize {
    double width;
    double height;
};

} // namespace

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
        || windowInfo.subsystem != SDL_SYSWM_COCOA) {
        return;
    }

    auto cocoaWindow = reinterpret_cast<id>(windowInfo.info.cocoa.window);
    const SEL setContentAspectRatio =
        sel_registerName("setContentAspectRatio:");
    const CocoaSize aspectRatio{
        static_cast<double>(logicalWidth),
        static_cast<double>(logicalHeight),
    };

    using SetContentAspectRatio = void (*)(id, SEL, CocoaSize);
    reinterpret_cast<SetContentAspectRatio>(objc_msgSend)(
        cocoaWindow,
        setContentAspectRatio,
        aspectRatio
    );
}

} // namespace Engine::ResizeHandler
