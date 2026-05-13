#pragma once

#include "../engine/scene/node.hpp"

#include <string>

/** Renders an image texture or a rectangular region from an image texture. */
class Texture : public Engine::Node {
  public:
    Texture();

    /** Register the Texture XML node type. */
    static void registerType();

    /** Apply path, region, and position properties from XML. */
    void
    setProperty(const std::string &name, const std::string &value) override;

  private:
    SDL_Point getPosition() const override;

    void renderSelf(SDL_Renderer *renderer);

    std::string path;
    SDL_Point position{0, 0};
    SDL_Rect region{0, 0, 0, 0};
};
