#pragma once

#include "../../resources/types/image_texture.hpp"
#include "../../runtime/game.hpp"
#include "../../runtime/render/canvas.hpp"
#include "object.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>

namespace Engine::Nodes {

/** Render an image texture resource into the node tree */
class Image : public Engine::Nodes::Object {
  public:
    Image() {
        this->declareHook(Engine::Nodes::Hook::Render);
        this->declareProperty(
            "path",
            this->path,
            [this](const std::string &value) { this->update(value); }
        );
        this->declareProperty("region", this->region);
    }

    /** Draw the selected texture region at local origin */
    void render(Engine::Render::Canvas &canvas) override {
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
            0,
            0,
            textureRegion.w,
            textureRegion.h,
        };

        canvas.copy(image.handle.get(), &textureRegion, destination);
    }

  private:
    /** Update the cached texture ID when the image path changes */
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

    std::string path;
    SDL_Rect region{0, 0, 0, 0};
    Engine::Resource::ID textureResourceID = 0;
};

} // namespace Engine::Nodes
