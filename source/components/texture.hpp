#pragma once

#include "../engine/node.hpp"

#include <string>

class Texture : public Node {
  public:
    Texture();

    static void registerType();

    void setProperty(
            const std::string &name, const std::string &value) override;

  private:
    SDL_Point getPosition() const override;

    void renderSelf(SDL_Renderer *renderer);

    std::string path;
    SDL_Point position{0, 0};
    SDL_Rect region{0, 0, 0, 0};
};
