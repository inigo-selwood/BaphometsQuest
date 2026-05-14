#pragma once

#include "../node/objectNode.hpp"

/** Simple filled rectangle node with colour, size, and position properties. */
class Box : public Engine::ObjectNode {
  public:
    Box();

    /** Register the generic Box XML node type. */
    static void registerType();

  protected:
    /** Return true when a point in logical render coordinates hits the box. */
    bool isPointInside(SDL_Point point) const;

  private:
    void renderSelf(SDL_Renderer *renderer);

    SDL_Color colour{255, 255, 255, 255};
};
