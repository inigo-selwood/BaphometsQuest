#include "sprite.hpp"

#include "../../resources/types/image_texture.hpp"
#include "../../runtime/game.hpp"
#include "../../utils/parse.hpp"

#include <tinyxml2.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace Engine::Nodes {

Sprite::Sprite() {
    this->declareHook(Engine::Nodes::Hook::Process);
    this->declareHook(Engine::Nodes::Hook::Render);
    this->declareProperty(
        "path",
        this->path,
        [this](const std::string &value) { this->update(value); }
    );
    this->declareProperty(
        "animations",
        this->animations,
        [this](const std::vector<Animation> &value) {
            this->updateAnimations(value);
        }
    );
    this->declareProperty(
        "animation",
        this->animation,
        [this](const std::string &value) { this->updateAnimation(value); }
    );
}

bool Sprite::loadXmlChildren(const tinyxml2::XMLElement &element) {
    std::vector<Animation> animations;

    for(const tinyxml2::XMLElement *animationElement =
            element.FirstChildElement();
        animationElement != nullptr;
        animationElement = animationElement->NextSiblingElement()) {
        animations.push_back(parseAnimation(*animationElement));
    }

    if(!animations.empty()) {
        this->setProperty("animations", animations);
    }

    return true;
}

void Sprite::process(float deltaSeconds) {
    if(this->animation.empty()) {
        return;
    }

    const Animation &activeAnimation = this->getAnimation();

    if(activeAnimation.frames.empty()) {
        return;
    }

    this->frameElapsed += deltaSeconds;

    while(this->frameElapsed
        >= activeAnimation.frames[this->frameIndex].duration) {
        const float duration =
            activeAnimation.frames[this->frameIndex].duration;

        if(duration <= 0.0F) {
            this->frameElapsed = 0.0F;
            return;
        }

        this->frameElapsed -= duration;
        this->frameIndex =
            (this->frameIndex + 1) % activeAnimation.frames.size();
    }
}

void Sprite::render(SDL_Renderer &renderer) {
    Engine::Game &game = this->getGame();

    if(this->textureResourceID == 0 || this->animation.empty()) {
        return;
    }

    const Animation &activeAnimation = this->getAnimation();

    if(activeAnimation.frames.empty()) {
        return;
    }

    const SDL_Rect region = activeAnimation.frames[this->frameIndex].region;
    const SDL_Rect destination{
        this->position.x,
        this->position.y,
        region.w,
        region.h,
    };
    const Engine::Resource::ImageTexture &image =
        game.resources.get<Engine::Resource::ImageTexture>(
            this->textureResourceID
        );

    if(SDL_RenderCopy(&renderer, image.handle.get(), &region, &destination)
        != 0) {
        throw std::runtime_error(
            std::string("Failed to render sprite node: ") + SDL_GetError()
        );
    }
}

void Sprite::update(const std::string &path) {
    Engine::Game &game = this->getGame();
    const bool textureChanged = path != this->path;

    this->path = path;

    if(!textureChanged && this->textureResourceID != 0) {
        return;
    }

    if(this->path.empty()) {
        this->textureResourceID = 0;
        return;
    }

    if(game.renderer == nullptr) {
        throw std::runtime_error(
            "Sprite requires a renderer before rebuilding texture"
        );
    }

    this->textureResourceID =
        game.resources.load<Engine::Resource::ImageTexture>(
            game.renderer.get(),
            this->path
        );
}

void Sprite::updateAnimations(const std::vector<Animation> &animations) {
    this->animations = animations;
    this->frameIndex = 0;
    this->frameElapsed = 0.0F;
}

void Sprite::updateAnimation(const std::string &animation) {
    this->animation = animation;
    this->frameIndex = 0;
    this->frameElapsed = 0.0F;
}

const Sprite::Animation &Sprite::getAnimation() const {
    for(const Animation &candidate : this->animations) {
        if(candidate.name == this->animation) {
            return candidate;
        }
    }

    throw std::runtime_error(
        "Unknown sprite animation '" + this->animation + "'"
    );
}

Sprite::Animation
Sprite::parseAnimation(const tinyxml2::XMLElement &animationElement) {
    const std::string elementName = animationElement.Name();

    if(elementName != "animation") {
        throw std::runtime_error(
            "Sprite child element '" + elementName + "' must be <animation>"
        );
    }

    const char *nameAttribute = animationElement.Attribute("name");

    if(nameAttribute == nullptr || std::string{nameAttribute}.empty()) {
        throw std::runtime_error(
            "Sprite animation requires a non-empty name attribute"
        );
    }

    Animation animation{
        nameAttribute,
        {},
    };

    for(const tinyxml2::XMLElement *frameElement =
            animationElement.FirstChildElement();
        frameElement != nullptr;
        frameElement = frameElement->NextSiblingElement()) {
        animation.frames.push_back(parseFrame(*frameElement));
    }

    if(animation.frames.empty()) {
        throw std::runtime_error(
            "Sprite animation '" + animation.name
            + "' must contain at least one frame"
        );
    }

    return animation;
}

Sprite::Frame Sprite::parseFrame(const tinyxml2::XMLElement &frameElement) {
    const std::string elementName = frameElement.Name();

    if(elementName != "frame") {
        throw std::runtime_error(
            "Sprite animation child element '" + elementName
            + "' must be <frame>"
        );
    }

    const char *regionAttribute = frameElement.Attribute("region");
    const char *durationAttribute = frameElement.Attribute("duration");

    if(regionAttribute == nullptr) {
        throw std::runtime_error("Sprite frame requires a region attribute");
    }

    if(durationAttribute == nullptr) {
        throw std::runtime_error("Sprite frame requires a duration attribute");
    }

    std::size_t parsedCharacters = 0;
    const std::string durationText = durationAttribute;
    const float duration = std::stof(durationText, &parsedCharacters);

    if(parsedCharacters != durationText.size()) {
        throw std::runtime_error(
            "Sprite frame duration is invalid: '" + durationText + "'"
        );
    }

    return Frame{
        Engine::Parse::rect(regionAttribute),
        duration,
    };
}

} // namespace Engine::Nodes
