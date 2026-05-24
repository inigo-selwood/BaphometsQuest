#pragma once

#include "../../../engine/nodes/native/image.hpp"

#include <SDL.h>

#include <string>
#include <unordered_map>

namespace tinyxml2 {
class XMLElement;
}

namespace Scenes::Play::Components {

/** Image node that maps named select modes to atlas regions from XML */
class SelectModeIcon : public Engine::Nodes::Image {
  public:
    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    /** Set the active icon mode */
    void setMode(const std::string &name);

  private:
    static std::pair<std::string, SDL_Rect>
    parseMode(const tinyxml2::XMLElement &modeElement);

    std::unordered_map<std::string, SDL_Rect> modes;
};

} // namespace Scenes::Play::Components
