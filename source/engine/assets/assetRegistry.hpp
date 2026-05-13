#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <memory>
#include <string>
#include <unordered_map>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>
#include <yaml-cpp/yaml.h>

namespace Engine {

/**
 * Caches runtime assets owned by the Game instance.
 *
 * Fonts, textures, generated text textures, and parsed data files are loaded
 * lazily and reused by key until clear() is called.
 */
class AssetRegistry {
  public:
    /** Borrowed SDL texture plus its pixel size. */
    struct TextureAsset {
        SDL_Texture *texture = nullptr;
        SDL_Point size{0, 0};
    };

    /** Destroy all cached fonts and textures. */
    void clear();

    /** Load or fetch a cached TTF font. */
    TTF_Font *getFont(const std::string &path, int size);

    /** Load or fetch a cached image texture. */
    const TextureAsset &
    getImageTexture(SDL_Renderer *renderer, const std::string &path);

    /** Load or fetch a cached XML document. */
    const tinyxml2::XMLDocument &
    getXML(const std::string &path, const std::string &context);

    /** Load or fetch a cached texture generated from rendered text. */
    const TextureAsset &getTextTexture(
        SDL_Renderer *renderer,
        const std::string &fontPath,
        int fontSize,
        SDL_Color colour,
        const std::string &text
    );

    /** Load or fetch a cached YAML document. */
    const YAML::Node &
    getYAML(const std::string &path, const std::string &context);

    /** Resolve a resource path against bundled resources when possible. */
    std::string resolvePath(const std::string &path) const;

  private:
    struct FontDeleter {
        void operator()(TTF_Font *font) const {
            TTF_CloseFont(font);
        }
    };

    struct TextureDeleter {
        void operator()(SDL_Texture *texture) const {
            spdlog::debug("Destroying cached texture.");
            SDL_DestroyTexture(texture);
        }
    };

    struct TextureEntry {
        std::unique_ptr<SDL_Texture, TextureDeleter> texture;
        SDL_Point size{0, 0};
    };

    static std::string makeFontKey(const std::string &path, int size);
    static std::string makeTextTextureKey(
        const std::string &fontPath,
        int fontSize,
        SDL_Color colour,
        const std::string &text
    );

    std::unordered_map<std::string, std::unique_ptr<TTF_Font, FontDeleter>>
        fonts;
    TextureAsset reusableTextureAsset;
    std::unordered_map<std::string, TextureEntry> textures;
    std::unordered_map<std::string, std::unique_ptr<tinyxml2::XMLDocument>>
        XMLDocuments;
    std::unordered_map<std::string, YAML::Node> YAMLDocuments;
};

} // namespace Engine
