#pragma once

#include "../assets/assetRegistry.hpp"
#include "../node/objectNode.hpp"

#include <optional>
#include <string>

/** Rendered text node backed by cached SDL_ttf textures. */
class Text : public Engine::ObjectNode {
  public:
    enum class Alignment {
        Bottom,
        Middle,
        Top,
    };

    enum class Justification {
        Center,
        Left,
        Right,
    };

    Text();

    /** Register the Text XML node type. */
    static void registerType();

  private:
    static std::string formatAlignment(Alignment value);
    static std::string formatJustification(Justification value);
    static Alignment parseAlignment(const std::string &value);
    static Justification parseJustification(const std::string &value);

    SDL_Rect getContentDestination(
        const SDL_Point &position,
        const SDL_Point &textureSize
    ) const;
    SDL_Rect getLayoutDestination(
        const SDL_Point &position,
        const SDL_Point &textureSize
    ) const;
    SDL_Point getLayoutSize(const SDL_Point &textureSize) const;

    void renderSelf(SDL_Renderer *renderer);
    void rebuildTexture(SDL_Renderer *renderer);

    Alignment alignment = Alignment::Top;
    Justification justification = Justification::Left;

    std::optional<SDL_Color> background;
    SDL_Color colour{255, 255, 255, 255};

    std::string fontPath;
    int fontSize = 8;
    std::string text;

    bool textureDirty = true;
    std::optional<Engine::AssetRegistry::AssetID> textureID;
};
