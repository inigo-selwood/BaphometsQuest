#include "resize_handler.hpp"

#include <algorithm>
#include <cmath>

#ifndef BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK
#define BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK 0
#endif

#ifndef BAPHOMETS_QUEST_PLATFORM_MACOS
#define BAPHOMETS_QUEST_PLATFORM_MACOS 0
#endif

#ifndef BAPHOMETS_QUEST_PLATFORM_WINDOWS
#define BAPHOMETS_QUEST_PLATFORM_WINDOWS 0
#endif

#ifndef BAPHOMETS_QUEST_PLATFORM_X11
#define BAPHOMETS_QUEST_PLATFORM_X11 0
#endif

#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK && BAPHOMETS_QUEST_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <unordered_map>
#endif

#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK                                        \
    && (BAPHOMETS_QUEST_PLATFORM_MACOS || BAPHOMETS_QUEST_PLATFORM_WINDOWS    \
        || BAPHOMETS_QUEST_PLATFORM_X11)
#include <SDL_syswm.h>
#endif

#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK && BAPHOMETS_QUEST_PLATFORM_MACOS
#include <objc/message.h>
#include <objc/runtime.h>
#endif

#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK && BAPHOMETS_QUEST_PLATFORM_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

namespace Engine::ResizeHandler {

namespace {

bool isResizeEvent(SDL_Window *window, const SDL_Event &event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.windowID == SDL_GetWindowID(window)
        && (event.window.event == SDL_WINDOWEVENT_RESIZED
            || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED);
}

#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK && BAPHOMETS_QUEST_PLATFORM_MACOS
struct CocoaSize {
    double width;
    double height;
};
#endif

#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK && BAPHOMETS_QUEST_PLATFORM_WINDOWS
struct WindowAspectLock {
    WNDPROC originalProcedure;
    int logicalWidth;
    int logicalHeight;
    int frameWidth;
    int frameHeight;
};

std::unordered_map<HWND, WindowAspectLock> windowAspectLocks;

LRESULT CALLBACK aspectWindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    auto stateIterator = windowAspectLocks.find(window);

    if(stateIterator == windowAspectLocks.end()) {
        return DefWindowProc(window, message, wParam, lParam);
    }

    WindowAspectLock &state = stateIterator->second;

    if(message == WM_SIZING && lParam != 0) {
        auto *rectangle = reinterpret_cast<RECT *>(lParam);

        const int windowWidth = rectangle->right - rectangle->left;
        const int windowHeight = rectangle->bottom - rectangle->top;
        const int contentWidth = std::max(1, windowWidth - state.frameWidth);
        const int contentHeight =
            std::max(1, windowHeight - state.frameHeight);
        const double targetAspect = static_cast<double>(state.logicalWidth)
            / static_cast<double>(state.logicalHeight);
        const double contentAspect = static_cast<double>(contentWidth)
            / static_cast<double>(contentHeight);

        int targetWindowWidth = windowWidth;
        int targetWindowHeight = windowHeight;

        const auto applyWidth = [&]() {
            if(wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT
                || wParam == WMSZ_BOTTOMLEFT) {
                rectangle->left = rectangle->right - targetWindowWidth;
            } else if(wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT
                || wParam == WMSZ_BOTTOMRIGHT) {
                rectangle->right = rectangle->left + targetWindowWidth;
            } else {
                const int centreX = rectangle->left
                    + ((rectangle->right - rectangle->left) / 2);
                rectangle->left = centreX - (targetWindowWidth / 2);
                rectangle->right = rectangle->left + targetWindowWidth;
            }
        };

        const auto applyHeight = [&]() {
            if(wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT
                || wParam == WMSZ_TOPRIGHT) {
                rectangle->top = rectangle->bottom - targetWindowHeight;
            } else if(wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT
                || wParam == WMSZ_BOTTOMRIGHT) {
                rectangle->bottom = rectangle->top + targetWindowHeight;
            } else {
                const int centreY = rectangle->top
                    + ((rectangle->bottom - rectangle->top) / 2);
                rectangle->top = centreY - (targetWindowHeight / 2);
                rectangle->bottom = rectangle->top + targetWindowHeight;
            }
        };

        if(wParam == WMSZ_LEFT || wParam == WMSZ_RIGHT) {
            targetWindowHeight =
                static_cast<int>(std::round(contentWidth / targetAspect))
                + state.frameHeight;
            applyHeight();
        } else if(wParam == WMSZ_TOP || wParam == WMSZ_BOTTOM) {
            targetWindowWidth =
                static_cast<int>(std::round(contentHeight * targetAspect))
                + state.frameWidth;
            applyWidth();
        } else if(contentAspect > targetAspect) {
            targetWindowHeight =
                static_cast<int>(std::round(contentWidth / targetAspect))
                + state.frameHeight;
            applyHeight();
        } else {
            targetWindowWidth =
                static_cast<int>(std::round(contentHeight * targetAspect))
                + state.frameWidth;
            applyWidth();
        }

        return TRUE;
    }

    const WNDPROC originalProcedure = state.originalProcedure;

    if(message == WM_NCDESTROY) {
        windowAspectLocks.erase(stateIterator);
        SetWindowLongPtr(
            window,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(originalProcedure)
        );
    }

    return CallWindowProc(originalProcedure, window, message, wParam, lParam);
}
#endif

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

void lockAspectRatio(SDL_Window *window, SDL_Renderer *renderer) {
#if BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK
    int logicalWidth = 0;
    int logicalHeight = 0;
    SDL_RenderGetLogicalSize(renderer, &logicalWidth, &logicalHeight);

    if(logicalWidth <= 0 || logicalHeight <= 0) {
        return;
    }

    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);

#if BAPHOMETS_QUEST_PLATFORM_MACOS
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
#elif BAPHOMETS_QUEST_PLATFORM_WINDOWS
    if(SDL_GetWindowWMInfo(window, &windowInfo) != SDL_TRUE
        || windowInfo.subsystem != SDL_SYSWM_WINDOWS) {
        return;
    }

    HWND nativeWindow = windowInfo.info.win.window;
    RECT windowRectangle;
    RECT clientRectangle;

    if(nativeWindow == nullptr
        || GetWindowRect(nativeWindow, &windowRectangle) == 0
        || GetClientRect(nativeWindow, &clientRectangle) == 0) {
        return;
    }

    const int frameWidth = (windowRectangle.right - windowRectangle.left)
        - (clientRectangle.right - clientRectangle.left);
    const int frameHeight = (windowRectangle.bottom - windowRectangle.top)
        - (clientRectangle.bottom - clientRectangle.top);
    auto stateIterator = windowAspectLocks.find(nativeWindow);

    if(stateIterator == windowAspectLocks.end()) {
        const auto originalProcedure = reinterpret_cast<WNDPROC>(
            GetWindowLongPtr(nativeWindow, GWLP_WNDPROC)
        );

        if(originalProcedure == nullptr) {
            return;
        }

        windowAspectLocks.emplace(
            nativeWindow,
            WindowAspectLock{
                originalProcedure,
                logicalWidth,
                logicalHeight,
                frameWidth,
                frameHeight,
            }
        );

        SetWindowLongPtr(
            nativeWindow,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(aspectWindowProcedure)
        );
    } else {
        stateIterator->second.logicalWidth = logicalWidth;
        stateIterator->second.logicalHeight = logicalHeight;
        stateIterator->second.frameWidth = frameWidth;
        stateIterator->second.frameHeight = frameHeight;
    }
#elif BAPHOMETS_QUEST_PLATFORM_X11
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
#endif
#else
    (void)window;
    (void)renderer;
#endif
}

} // namespace Engine::ResizeHandler
