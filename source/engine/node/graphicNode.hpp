#pragma once

#include "node.hpp"

#include <optional>

namespace Engine {

/** Node with local visual properties such as position, size, and visibility.
 */
class GraphicNode : public Node {
  public:
    GraphicNode();

    /** Return the local size if one has been configured. */
    std::optional<SDL_Point> getSize() const;

    /** Return true when this node should render. */
    bool isVisible() const;

    /** Render this node and children when visible. */
    void render(SDL_Renderer *renderer) override;

  protected:
    /** Return the local position. */
    SDL_Point getPosition() const override;

  private:
    SDL_Point position{0, 0};
    std::optional<SDL_Point> size;
    bool visible = true;
};

} // namespace Engine
