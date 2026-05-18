#include "../handler.hpp"

#if !BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK || !BAPHOMETS_QUEST_PLATFORM_WINDOWS
#error                                                                        \
    "resize/windows.cpp should only be compiled for native Windows aspect locking"
#endif

#include <windows.h>

#include <SDL_syswm.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace Engine::ResizeHandler {

namespace {

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
}

} // namespace Engine::ResizeHandler
