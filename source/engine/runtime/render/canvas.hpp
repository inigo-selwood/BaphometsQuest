#pragma once

#include "context.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>

namespace Engine::Render {

/** SDL renderer wrapper that applies render context transforms */
class Canvas {
  public:
    Canvas(SDL_Renderer &renderer, const Engine::Render::Context &context)
        : renderer(renderer), context(context) {}

    /** Return the raw renderer for API surfaces that still need it */
    SDL_Renderer &getRenderer() const {
        return this->renderer;
    }

    /** Return the inherited render context */
    const Engine::Render::Context &getContext() const {
        return this->context;
    }

    /** Convert a local rectangle into final screen coordinates */
    SDL_Rect toScreen(SDL_Rect rect) const {
        rect.x += this->context.origin.x;
        rect.y += this->context.origin.y;

        if(this->context.mode == Engine::Render::CanvasMode::World) {
            rect.x -= this->context.viewportOrigin.x;
            rect.y -= this->context.viewportOrigin.y;
        }

        return rect;
    }

    /** Return true when a local rectangle intersects the screen */
    bool isVisible(SDL_Rect rect) const {
        const SDL_Rect screenRect = this->toScreen(rect);

        return SDL_HasIntersection(&screenRect, &this->context.screenBounds)
            == SDL_TRUE;
    }

    /** Set the renderer blend mode */
    void setBlendMode(SDL_BlendMode blendMode) const {
        if(SDL_SetRenderDrawBlendMode(&this->renderer, blendMode) != 0) {
            throw std::runtime_error(
                std::string("Failed to set canvas blend mode: ")
                + SDL_GetError()
            );
        }
    }

    /** Set the renderer draw colour */
    void setDrawColour(SDL_Color colour) const {
        if(SDL_SetRenderDrawColor(
               &this->renderer,
               colour.r,
               colour.g,
               colour.b,
               colour.a
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to set canvas draw colour: ")
                + SDL_GetError()
            );
        }
    }

    /** Copy a texture into a local destination rectangle */
    void copy(
        SDL_Texture *texture,
        const SDL_Rect *source,
        SDL_Rect destination
    ) const {
        const SDL_Rect screenDestination = this->toScreen(destination);

        if(SDL_RenderCopy(&this->renderer, texture, source, &screenDestination)
            != 0) {
            throw std::runtime_error(
                std::string("Failed to copy canvas texture: ") + SDL_GetError()
            );
        }
    }

    /** Fill a local rectangle */
    void fillRect(SDL_Rect rectangle) const {
        const SDL_Rect screenRectangle = this->toScreen(rectangle);

        if(SDL_RenderFillRect(&this->renderer, &screenRectangle) != 0) {
            throw std::runtime_error(
                std::string("Failed to fill canvas rectangle: ")
                + SDL_GetError()
            );
        }
    }

  private:
    SDL_Renderer &renderer;
    Engine::Render::Context context;
};

} // namespace Engine::Render
