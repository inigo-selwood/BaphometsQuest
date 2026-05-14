#include "objectNode.hpp"

namespace Engine {

SDL_Point ObjectNode::getWorldPosition() const {
    return this->getGlobalPosition();
}

} // namespace Engine
