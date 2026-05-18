#pragma once

#include "../../resources/types/image_texture.hpp"
#include "../../runtime/game.hpp"
#include "../base.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>

namespace Engine::Nodes {

/** Render an image texture resource into the node tree */
class Image : public Engine::Nodes::Base {
  public:
    Image() {
        this->declareHook(Engine::Nodes::Hook::Render);
        this->declareProperty(
            "path",
            this->path,
            [this](const std::string &value) { this->update(value); }
        );
        this->declareProperty("position", this->position);
        this->declareProperty("region", this->region);
    }

    void render(SDL_Renderer &renderer) override {
        Engine::Game &game = this->getGame();

        if(this->textureResourceID == 0) {
            return;
        }

        const Engine::Resource::ImageTexture &image =
            game.resources.get<Engine::Resource::ImageTexture>(
                this->textureResourceID
            );
        SDL_Rect textureRegion = image.size;

        if(this->region.w > 0 && this->region.h > 0) {
            textureRegion = this->region;
        }

        const SDL_Rect destination{
            this->position.x,
            this->position.y,
            textureRegion.w,
            textureRegion.h,
        };

        if(SDL_RenderCopy(
               &renderer,
               image.handle.get(),
               &textureRegion,
               &destination
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to render image node: ") + SDL_GetError()
            );
        }
    }

  private:
    void update(const std::string &path) {
        Engine::Game &game = this->getGame();
        const bool textureChanged = path != this->path;

        this->path = path;

        if(!textureChanged && this->textureResourceID != 0) {
            return;
        }

        if(this->path.empty()) {
            this->textureResourceID = 0;
            return;
        }

        if(game.renderer == nullptr) {
            throw std::runtime_error(
                "Image requires a renderer before rebuilding texture"
            );
        }

        this->textureResourceID =
            game.resources.load<Engine::Resource::ImageTexture>(
                game.renderer.get(),
                this->path
            );
    }

    Engine::Resource::ID textureResourceID = 0;
    std::string path;
    SDL_Point position{0, 0};
    SDL_Rect region{0, 0, 0, 0};
};

} // namespace Engine::Nodes
