#pragma once

#include "../../resources/base.hpp"
#include "object.hpp"

#include <SDL.h>

#include <cstddef>
#include <string>
#include <vector>

namespace tinyxml2 {
class XMLElement;
}

namespace Engine::Nodes {

/** Render a named animation frame from a texture atlas */
class Sprite : public Engine::Nodes::Object {
  public:
    /** Single animation frame drawn from an atlas region */
    struct Frame {
        SDL_Rect region;
        float duration = 0.0F;
    };

    /** Named animation made from timed frames */
    struct Animation {
        std::string name;
        std::vector<Frame> frames;
    };

    Sprite();

    /** Load animation definitions from XML child elements */
    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    /** Advance the active animation by elapsed frame time */
    void process(float deltaSeconds) override;

    /** Draw the current animation frame */
    void render(Engine::Render::Canvas &canvas) override;

  private:
    /** Parse one XML animation child */
    static Animation
    parseAnimation(const tinyxml2::XMLElement &animationElement);

    /** Parse one XML frame child */
    static Frame parseFrame(const tinyxml2::XMLElement &frameElement);

    /** Return the active animation definition */
    const Animation &getAnimation() const;

    /** Update the cached texture ID when the sprite path changes */
    void update(const std::string &path);

    /** Reset animation timing when changing active animation */
    void updateAnimation(const std::string &animation);

    /** Reset animation timing when animation definitions change */
    void updateAnimations(const std::vector<Animation> &animations);

    std::string animation;
    std::vector<Animation> animations;
    float frameElapsed = 0.0F;
    std::size_t frameIndex = 0;

    std::string path;
    Engine::Resource::ID textureResourceID = 0;
};

} // namespace Engine::Nodes
