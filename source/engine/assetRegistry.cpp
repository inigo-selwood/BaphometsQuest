#include "assetRegistry.hpp"

#include <stdexcept>

void AssetRegistry::clear() {
    spdlog::debug("Clearing {} cached font(s).", fonts().size());
    spdlog::debug("Clearing {} cached texture(s).", textures().size());

    fonts().clear();
    textures().clear();
    reusableTextureAsset = TextureAsset{};
}

TTF_Font *AssetRegistry::getFont(const std::string &path, int size) {
    const std::string key = makeFontKey(path, size);
    auto &fontCache = fonts();
    const auto fontIterator = fontCache.find(key);

    if (fontIterator != fontCache.end()) {
        return fontIterator->second.get();
    }

    spdlog::debug("Loading font '{}' at size {}.", path, size);

    std::unique_ptr<TTF_Font, FontDeleter> font(
            TTF_OpenFont(path.c_str(), size));

    if (font == nullptr) {
        throw std::runtime_error(std::string("Failed to load font '") + path
                + "': " + TTF_GetError());
    }

    auto *fontReference = font.get();
    fontCache.emplace(key, std::move(font));

    return fontReference;
}

const AssetRegistry::TextureAsset &AssetRegistry::getImageTexture(
        SDL_Renderer *renderer, const std::string &path) {
    auto &textureCache = textures();
    const auto textureIterator = textureCache.find(path);

    if (textureIterator != textureCache.end()) {
        reusableTextureAsset = TextureAsset{
                textureIterator->second.texture.get(),
                textureIterator->second.size,
        };
        return reusableTextureAsset;
    }

    spdlog::debug("Loading image texture '{}'.", path);

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
            IMG_Load(path.c_str()), SDL_FreeSurface);

    if (surface == nullptr) {
        throw std::runtime_error(std::string("Failed to load image '") + path
                + "': " + IMG_GetError());
    }

    TextureEntry entry;
    entry.size = SDL_Point{surface->w, surface->h};
    entry.texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if (entry.texture == nullptr) {
        throw std::runtime_error(
                std::string("Failed to create image texture '") + path
                + "': " + SDL_GetError());
    }

    auto *texture = entry.texture.get();
    const SDL_Point size = entry.size;
    textureCache.emplace(path, std::move(entry));

    reusableTextureAsset = TextureAsset{texture, size};
    return reusableTextureAsset;
}

const AssetRegistry::TextureAsset &AssetRegistry::getTextTexture(
        SDL_Renderer *renderer, const std::string &fontPath, int fontSize,
        SDL_Color colour, const std::string &text) {
    const std::string key =
            makeTextTextureKey(fontPath, fontSize, colour, text);
    auto &textureCache = textures();
    const auto textureIterator = textureCache.find(key);

    if (textureIterator != textureCache.end()) {
        reusableTextureAsset = TextureAsset{
                textureIterator->second.texture.get(),
                textureIterator->second.size,
        };
        return reusableTextureAsset;
    }

    spdlog::debug("Creating text texture '{}' from '{}' at size {}.",
            text,
            fontPath,
            fontSize);

    auto *font = getFont(fontPath, fontSize);

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
            TTF_RenderUTF8_Blended(font, text.c_str(), colour),
            SDL_FreeSurface);

    if (surface == nullptr) {
        throw std::runtime_error(std::string("Failed to render text '") + text
                + "': " + TTF_GetError());
    }

    TextureEntry entry;
    entry.size = SDL_Point{surface->w, surface->h};
    entry.texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if (entry.texture == nullptr) {
        throw std::runtime_error(std::string("Failed to create text texture: ")
                + SDL_GetError());
    }

    auto *texture = entry.texture.get();
    const SDL_Point size = entry.size;
    textureCache.emplace(key, std::move(entry));

    reusableTextureAsset = TextureAsset{texture, size};
    return reusableTextureAsset;
}

std::unordered_map<std::string,
        std::unique_ptr<TTF_Font, AssetRegistry::FontDeleter>> &
AssetRegistry::fonts() {
    static std::unordered_map<std::string,
            std::unique_ptr<TTF_Font, FontDeleter>>
            fontCache;
    return fontCache;
}

std::string AssetRegistry::makeFontKey(const std::string &path, int size) {
    return path + "#" + std::to_string(size);
}

std::string AssetRegistry::makeTextTextureKey(const std::string &fontPath,
        int fontSize, SDL_Color colour, const std::string &text) {
    return fontPath + "#" + std::to_string(fontSize) + "#"
            + std::to_string(colour.r) + "," + std::to_string(colour.g) + ","
            + std::to_string(colour.b) + "," + std::to_string(colour.a) + "#"
            + text;
}

std::unordered_map<std::string, AssetRegistry::TextureEntry> &
AssetRegistry::textures() {
    static std::unordered_map<std::string, TextureEntry> textureCache;
    return textureCache;
}
