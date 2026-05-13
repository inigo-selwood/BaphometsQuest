#pragma once

#include "../engine/scene/node.hpp"

/** Simple filled rectangle node with colour, size, and position properties. */
class Box : public Engine::Node {
  public:
    Box();

    /** Register the generic Box XML node type. */
    static void registerType();

    /** Set the fill colour programmatically. */
    void setColour(SDL_Color newColour);

    /** Apply colour, size, and position properties from XML. */
    void
    setProperty(const std::string &name, const std::string &value) override;

  private:
    SDL_Point getPosition() const override;

    void renderSelf(SDL_Renderer *renderer);

  protected:
    /** Return true when a point in logical render coordinates hits the box. */
    bool isPointInside(SDL_Point point) const;

  private:
    SDL_Color colour{255, 255, 255, 255};
    SDL_Point position{0, 0};
    SDL_Point size{0, 0};
};
