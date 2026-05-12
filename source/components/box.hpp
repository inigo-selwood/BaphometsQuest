#pragma once

#include "../engine/node.hpp"

class Box : public Node {
  public:
    Box() = default;

    static void registerType();

    void render(SDL_Renderer *renderer) override;

    void setProperty(
            const std::string &name, const std::string &value) override;

  private:
    SDL_Color colour;
    SDL_Point position;
    SDL_Point size;
};
