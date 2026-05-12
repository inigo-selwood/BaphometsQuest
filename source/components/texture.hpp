#pragma once

#include "../engine/node.hpp"

#include <string>

class Texture : public Node {
  public:
    Texture() = default;

    static void registerType();

    void render(SDL_Renderer *renderer) override;

    void setProperty(
            const std::string &name, const std::string &value) override;

  private:
    SDL_Point position{0, 0};
    SDL_Rect region{0, 0, 0, 0};
    std::string path;
};
