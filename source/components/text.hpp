#pragma once

#include "../engine/node.hpp"

#include <string>

class Text : public Node {
  public:
    Text();

    static void registerType();

    void setProperty(
            const std::string &name, const std::string &value) override;

  private:
    SDL_Point getPosition() const override;

    void renderSelf(SDL_Renderer *renderer);
    void rebuildTexture(SDL_Renderer *renderer);

    SDL_Color colour{255, 255, 255, 255};
    std::string fontPath;
    int fontSize = 8;
    SDL_Point position{0, 0};
    std::string text;
    SDL_Texture *texture = nullptr;
    bool textureDirty = true;
    SDL_Point textureSize{0, 0};
};
