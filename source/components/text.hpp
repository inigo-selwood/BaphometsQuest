#pragma once

#include "../engine/node.hpp"

#include <string>

class Text : public Node {
  public:
    Text() = default;

    static void registerType();

    void render(SDL_Renderer *renderer) override;

    void setProperty(
            const std::string &name, const std::string &value) override;

  private:
    void rebuildTexture(SDL_Renderer *renderer);

    SDL_Color colour{255, 255, 255, 255};
    SDL_Point position{0, 0};
    int fontSize = 8;
    bool textureDirty = true;
    std::string fontPath;
    std::string text;
    SDL_Texture *texture = nullptr;
    SDL_Point textureSize{0, 0};
};
