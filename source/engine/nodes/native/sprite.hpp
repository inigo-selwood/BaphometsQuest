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

    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    void process(float deltaSeconds) override;

    void render(SDL_Renderer &renderer) override;

  private:
    /** Update the cached texture ID when the sprite path changes */
    void update(const std::string &path);

    const Animation &getAnimation() const;

    /** Reset animation timing when animation definitions change */
    void updateAnimations(const std::vector<Animation> &animations);

    /** Reset animation timing when changing active animation */
    void updateAnimation(const std::string &animation);

    static Animation
    parseAnimation(const tinyxml2::XMLElement &animationElement);

    static Frame parseFrame(const tinyxml2::XMLElement &frameElement);

    Engine::Resource::ID textureResourceID = 0;
    std::string path;
    std::vector<Animation> animations;
    std::string animation;
    std::size_t frameIndex = 0;
    float frameElapsed = 0.0F;
};

} // namespace Engine::Nodes
