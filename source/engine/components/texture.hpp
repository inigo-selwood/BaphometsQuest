#pragma once

#include "../assets/assetRegistry.hpp"
#include "../node/objectNode.hpp"

#include <optional>
#include <string>

/** Renders an image texture or a rectangular region from an image texture. */
class Texture : public Engine::ObjectNode {
  public:
    Texture();

    /** Register the Texture XML node type. */
    static void registerType();

  private:
    void renderSelf(SDL_Renderer *renderer);

    SDL_Rect region{0, 0, 0, 0};

    std::string path;

    std::optional<Engine::AssetRegistry::AssetID> textureID;
};
