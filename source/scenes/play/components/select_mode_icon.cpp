#include "select_mode_icon.hpp"

#include "../../../engine/utils/parse.hpp"

#include <tinyxml2.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace Scenes::Play::Components {

bool SelectModeIcon::loadXmlChildren(const tinyxml2::XMLElement &element) {
    std::unordered_map<std::string, SDL_Rect> parsedModes;

    for(const tinyxml2::XMLElement *modeElement = element.FirstChildElement();
        modeElement != nullptr;
        modeElement = modeElement->NextSiblingElement()) {
        parsedModes.insert(parseMode(*modeElement));
    }

    if(parsedModes.empty()) {
        throw std::runtime_error("Select mode icon requires mode children");
    }

    this->modes = std::move(parsedModes);

    return true;
}

void SelectModeIcon::setMode(const std::string &name) {
    const auto mode = this->modes.find(name);

    if(mode == this->modes.end()) {
        throw std::runtime_error("Unknown select mode icon '" + name + "'");
    }

    this->setProperty("region", mode->second);
}

std::pair<std::string, SDL_Rect>
SelectModeIcon::parseMode(const tinyxml2::XMLElement &modeElement) {
    const std::string elementName = modeElement.Name();

    if(elementName != "mode") {
        throw std::runtime_error(
            "Select mode icon child element '" + elementName
            + "' must be <mode>"
        );
    }

    const char *nameAttribute = modeElement.Attribute("name");
    const char *regionAttribute = modeElement.Attribute("region");

    if(nameAttribute == nullptr || std::string{nameAttribute}.empty()) {
        throw std::runtime_error("Select mode icon mode requires a name");
    }

    if(regionAttribute == nullptr || std::string{regionAttribute}.empty()) {
        throw std::runtime_error("Select mode icon mode requires a region");
    }

    return {
        std::string{nameAttribute},
        Engine::Parse::rect(regionAttribute),
    };
}

} // namespace Scenes::Play::Components
