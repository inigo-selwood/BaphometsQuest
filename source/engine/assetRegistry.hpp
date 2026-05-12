#pragma once

#include "includes.hpp"

#include <memory>
#include <string>
#include <unordered_map>

class AssetRegistry {
  public:
    struct TextureAsset {
        SDL_Texture *texture = nullptr;
        SDL_Point size{0, 0};
    };

    void clear();
    TTF_Font *getFont(const std::string &path, int size);
    const TextureAsset &getImageTexture(
            SDL_Renderer *renderer, const std::string &path);
    const TextureAsset &getTextTexture(SDL_Renderer *renderer,
            const std::string &fontPath, int fontSize, SDL_Color colour,
            const std::string &text);

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

    static std::unordered_map<std::string,
            std::unique_ptr<TTF_Font, FontDeleter>> &
    fonts();
    static std::string makeFontKey(const std::string &path, int size);
    static std::string makeTextTextureKey(const std::string &fontPath,
            int fontSize, SDL_Color colour, const std::string &text);
    static std::unordered_map<std::string, TextureEntry> &textures();

    TextureAsset reusableTextureAsset;
};
