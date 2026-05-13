#include "assetRegistry.hpp"

#include <SDL.h>
#include <SDL_image.h>

#include <filesystem>
#include <memory>
#include <stdexcept>

namespace Engine {

namespace {

struct SDLBasePathDeleter {
    void operator()(char *path) const {
        SDL_free(path);
    }
};

bool pathExists(const std::filesystem::path &path) {
    std::error_code error;
    return std::filesystem::exists(path, error);
}

} // namespace

void AssetRegistry::clear() {
    spdlog::debug("Clearing {} cached font(s).", this->fonts.size());
    spdlog::debug("Clearing {} cached texture(s).", this->textures.size());
    spdlog::debug(
        "Clearing {} cached XML document(s).",
        this->XMLDocuments.size()
    );
    spdlog::debug(
        "Clearing {} cached YAML document(s).",
        this->YAMLDocuments.size()
    );

    this->fonts.clear();
    this->textures.clear();
    this->XMLDocuments.clear();
    this->YAMLDocuments.clear();
    this->reusableTextureAsset = TextureAsset{};
}

TTF_Font *AssetRegistry::getFont(const std::string &path, int size) {
    const std::string resolvedPath = this->resolvePath(path);
    const std::string key = AssetRegistry::makeFontKey(resolvedPath, size);
    auto &fontCache = this->fonts;
    const auto fontIterator = fontCache.find(key);

    if(fontIterator != fontCache.end()) {
        return fontIterator->second.get();
    }

    spdlog::debug("Loading font '{}' at size {}.", resolvedPath, size);

    std::unique_ptr<TTF_Font, FontDeleter> font(
        TTF_OpenFont(resolvedPath.c_str(), size)
    );

    if(font == nullptr) {
        throw std::runtime_error(
            std::string("Failed to load font '") + resolvedPath
            + "': " + TTF_GetError()
        );
    }

    auto *fontReference = font.get();
    fontCache.emplace(key, std::move(font));

    return fontReference;
}

const AssetRegistry::TextureAsset &AssetRegistry::getImageTexture(
    SDL_Renderer *renderer,
    const std::string &path
) {
    const std::string resolvedPath = this->resolvePath(path);
    auto &textureCache = this->textures;
    const auto textureIterator = textureCache.find(resolvedPath);

    if(textureIterator != textureCache.end()) {
        this->reusableTextureAsset = TextureAsset{
            textureIterator->second.texture.get(),
            textureIterator->second.size,
        };
        return this->reusableTextureAsset;
    }

    spdlog::debug("Loading image texture '{}'.", resolvedPath);

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
        IMG_Load(resolvedPath.c_str()),
        SDL_FreeSurface
    );

    if(surface == nullptr) {
        throw std::runtime_error(
            std::string("Failed to load image '") + resolvedPath
            + "': " + IMG_GetError()
        );
    }

    TextureEntry entry;
    entry.size = SDL_Point{surface->w, surface->h};
    entry.texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if(entry.texture == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create image texture '") + resolvedPath
            + "': " + SDL_GetError()
        );
    }

    auto *texture = entry.texture.get();
    const SDL_Point size = entry.size;
    textureCache.emplace(resolvedPath, std::move(entry));

    this->reusableTextureAsset = TextureAsset{texture, size};
    return this->reusableTextureAsset;
}

const tinyxml2::XMLDocument &
AssetRegistry::getXML(const std::string &path, const std::string &context) {
    const std::string resolvedPath = this->resolvePath(path);
    const auto XMLIterator = this->XMLDocuments.find(resolvedPath);

    if(XMLIterator != this->XMLDocuments.end()) {
        return *XMLIterator->second;
    }

    spdlog::info("Loading {} '{}'.", context, resolvedPath);

    auto document = std::make_unique<tinyxml2::XMLDocument>();
    const tinyxml2::XMLError loadResult =
        document->LoadFile(resolvedPath.c_str());

    if(loadResult != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Failed to load " + context + " '" + resolvedPath
            + "': " + document->ErrorStr()
        );
    }

    spdlog::info("Loaded {} '{}'.", context, resolvedPath);

    const auto [documentIterator, inserted] =
        this->XMLDocuments.emplace(resolvedPath, std::move(document));
    (void)inserted;

    return *documentIterator->second;
}

const AssetRegistry::TextureAsset &AssetRegistry::getTextTexture(
    SDL_Renderer *renderer,
    const std::string &fontPath,
    int fontSize,
    SDL_Color colour,
    const std::string &text
) {
    const std::string resolvedFontPath = this->resolvePath(fontPath);
    const std::string key = AssetRegistry::makeTextTextureKey(
        resolvedFontPath,
        fontSize,
        colour,
        text
    );
    auto &textureCache = this->textures;
    const auto textureIterator = textureCache.find(key);

    if(textureIterator != textureCache.end()) {
        this->reusableTextureAsset = TextureAsset{
            textureIterator->second.texture.get(),
            textureIterator->second.size,
        };
        return this->reusableTextureAsset;
    }

    spdlog::debug(
        "Creating text texture '{}' from '{}' at size {}.",
        text,
        resolvedFontPath,
        fontSize
    );

    auto *font = this->getFont(resolvedFontPath, fontSize);

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
        TTF_RenderUTF8_Blended(font, text.c_str(), colour),
        SDL_FreeSurface
    );

    if(surface == nullptr) {
        throw std::runtime_error(
            std::string("Failed to render text '") + text
            + "': " + TTF_GetError()
        );
    }

    TextureEntry entry;
    entry.size = SDL_Point{surface->w, surface->h};
    entry.texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if(entry.texture == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create text texture: ") + SDL_GetError()
        );
    }

    auto *texture = entry.texture.get();
    const SDL_Point size = entry.size;
    textureCache.emplace(key, std::move(entry));

    this->reusableTextureAsset = TextureAsset{texture, size};
    return this->reusableTextureAsset;
}

const YAML::Node &
AssetRegistry::getYAML(const std::string &path, const std::string &context) {
    const std::string resolvedPath = this->resolvePath(path);
    const auto YAMLIterator = this->YAMLDocuments.find(resolvedPath);

    if(YAMLIterator != this->YAMLDocuments.end()) {
        return YAMLIterator->second;
    }

    spdlog::info("Loading {} '{}'.", context, resolvedPath);

    try {
        auto [documentIterator, inserted] = this->YAMLDocuments.emplace(
            resolvedPath,
            YAML::LoadFile(resolvedPath)
        );
        (void)inserted;

        spdlog::info("Loaded {} '{}'.", context, resolvedPath);

        return documentIterator->second;
    } catch(const YAML::Exception &exception) {
        throw std::runtime_error(
            "Failed to load " + context + " '" + resolvedPath
            + "': " + exception.what()
        );
    }
}

std::string AssetRegistry::resolvePath(const std::string &path) const {
    const std::filesystem::path requestedPath(path);

    if(requestedPath.is_absolute()) {
        return path;
    }

    std::unique_ptr<char, SDLBasePathDeleter> basePath(SDL_GetBasePath());

    if(basePath != nullptr) {
        const auto bundledPath = std::filesystem::path(basePath.get()) / path;

        if(pathExists(bundledPath)) {
            return bundledPath.string();
        }
    }

    return path;
}

std::string AssetRegistry::makeFontKey(const std::string &path, int size) {
    return path + "#" + std::to_string(size);
}

std::string AssetRegistry::makeTextTextureKey(
    const std::string &fontPath,
    int fontSize,
    SDL_Color colour,
    const std::string &text
) {
    return fontPath + "#" + std::to_string(fontSize) + "#"
        + std::to_string(colour.r) + "," + std::to_string(colour.g) + ","
        + std::to_string(colour.b) + "," + std::to_string(colour.a) + "#"
        + text;
}

} // namespace Engine
