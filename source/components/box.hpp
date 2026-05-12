#pragma once

#include "../engine/node.hpp"

class Box : public Node {
  public:
    Box();

    static void registerType();

    void setColour(SDL_Color newColour);

    void setProperty(
            const std::string &name, const std::string &value) override;

  private:
    SDL_Point getPosition() const override;

    void renderSelf(SDL_Renderer *renderer);

  protected:
    bool isPointInside(SDL_Point point) const;

  private:
    SDL_Color colour{255, 255, 255, 255};
    SDL_Point position{0, 0};
    SDL_Point size{0, 0};
};
