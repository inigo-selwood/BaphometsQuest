#pragma once

#include "../engine/scene/node.hpp"

#include <string>

/** Rendered text node backed by cached SDL_ttf textures. */
class Text : public Engine::Node {
  public:
    Text();

    /** Register the Text XML node type. */
    static void registerType();

    /** Apply text, font, colour, size, and position properties from XML. */
    void
    setProperty(const std::string &name, const std::string &value) override;

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
