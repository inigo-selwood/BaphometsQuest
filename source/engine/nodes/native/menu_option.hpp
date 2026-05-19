#pragma once

#include "label.hpp"

#include <SDL.h>

#include <string>

namespace Engine::Nodes {

/** Label-backed selectable menu option */
class MenuOption : public Engine::Nodes::Label {
  public:
    MenuOption() {
        this->declareProperty("tag", this->tag);
    }

    /** Return the value emitted when this option is selected */
    const std::string &getTag() const {
        return this->tag;
    }

  private:
    std::string tag;
};

} // namespace Engine::Nodes
