#include "assetRegistry.hpp"

#include <SDL.h>
#include <SDL_image.h>

#include <memory>
#include <stdexcept>

namespace Engine {

AssetRegistry::AssetID
AssetRegistry::loadFont(const std::string &path, int size) {
    const std::string resolvedPath = this->resolvePath(path);
    const std::string key = AssetRegistry::makeFontKey(resolvedPath, size);
    const AssetID id = this->getOrCreateID(AssetType::Font, key);
    auto &fontCache = this->fonts;
    const auto fontIterator = fontCache.find(key);

    if(fontIterator != fontCache.end()) {
        return id;
    }

    spdlog::debug("Loading font asset '{}'.", id.UID);

    std::unique_ptr<TTF_Font, FontDeleter> font(
        TTF_OpenFont(resolvedPath.c_str(), size)
    );

    if(font == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to load font '") + resolvedPath
            + "': " + TTF_GetError()
        );
    }

    fontCache.emplace(key, std::move(font));

    return id;
}

AssetRegistry::AssetID AssetRegistry::loadImageTexture(
    SDL_Renderer *renderer,
    const std::string &path
) {
    const std::string resolvedPath = this->resolvePath(path);
    const AssetID id =
        this->getOrCreateID(AssetType::ImageTexture, resolvedPath);
    auto &textureCache = this->textures;
    const auto textureIterator = textureCache.find(resolvedPath);

    if(textureIterator != textureCache.end()) {
        return id;
    }

    spdlog::debug("Loading image texture asset '{}'.", id.UID);

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
        IMG_Load(resolvedPath.c_str()),
        SDL_FreeSurface
    );

    if(surface == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to load image '") + resolvedPath
            + "': " + IMG_GetError()
        );
    }

    TextureEntry entry;
    entry.size = SDL_Point{surface->w, surface->h};
    entry.type = AssetType::ImageTexture;
    entry.texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if(entry.texture == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to create image texture '") + resolvedPath
            + "': " + SDL_GetError()
        );
    }

    textureCache.emplace(resolvedPath, std::move(entry));

    return id;
}

AssetRegistry::AssetID AssetRegistry::loadMusic(const std::string &path) {
    const std::string resolvedPath = this->resolvePath(path);
    const AssetID id = this->getOrCreateID(AssetType::Music, resolvedPath);
    auto &musicCache = this->music;
    const auto musicIterator = musicCache.find(resolvedPath);

    if(musicIterator != musicCache.end()) {
        return id;
    }

    spdlog::debug("Loading music asset '{}'.", id.UID);

    std::unique_ptr<Mix_Music, MusicDeleter> music(
        Mix_LoadMUS(resolvedPath.c_str())
    );

    if(music == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to load music '") + resolvedPath
            + "': " + Mix_GetError()
        );
    }

    musicCache.emplace(resolvedPath, std::move(music));

    return id;
}

AssetRegistry::AssetID
AssetRegistry::loadSoundEffect(const std::string &path) {
    const std::string resolvedPath = this->resolvePath(path);
    const AssetID id =
        this->getOrCreateID(AssetType::SoundEffect, resolvedPath);
    auto &soundEffectCache = this->soundEffects;
    const auto soundEffectIterator = soundEffectCache.find(resolvedPath);

    if(soundEffectIterator != soundEffectCache.end()) {
        return id;
    }

    spdlog::debug("Loading sound effect asset '{}'.", id.UID);

    std::unique_ptr<Mix_Chunk, SoundEffectDeleter> soundEffect(
        Mix_LoadWAV(resolvedPath.c_str())
    );

    if(soundEffect == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to load sound effect '") + resolvedPath
            + "': " + Mix_GetError()
        );
    }

    soundEffectCache.emplace(resolvedPath, std::move(soundEffect));

    return id;
}

AssetRegistry::AssetID
AssetRegistry::loadXML(const std::string &path, const std::string &context) {
    const std::string resolvedPath = this->resolvePath(path);
    const AssetID id = this->getOrCreateID(AssetType::XML, resolvedPath);
    const auto XMLIterator = this->XMLDocuments.find(resolvedPath);

    if(XMLIterator != this->XMLDocuments.end()) {
        return id;
    }

    spdlog::info("Loading XML asset '{}' for {}.", id.UID, context);

    auto document = std::make_unique<tinyxml2::XMLDocument>();
    const tinyxml2::XMLError loadResult =
        document->LoadFile(resolvedPath.c_str());

    if(loadResult != tinyxml2::XML_SUCCESS) {
        this->forgetAsset(id);
        throw std::runtime_error(
            "Failed to load " + context + " '" + resolvedPath
            + "': " + document->ErrorStr()
        );
    }

    spdlog::info("Loaded XML asset '{}' for {}.", id.UID, context);

    const auto [documentIterator, inserted] =
        this->XMLDocuments.emplace(resolvedPath, std::move(document));
    (void)documentIterator;
    (void)inserted;

    return id;
}

AssetRegistry::AssetID AssetRegistry::loadTextTexture(
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
    const AssetID id = this->getOrCreateID(AssetType::TextTexture, key);
    auto &textureCache = this->textures;
    const auto textureIterator = textureCache.find(key);

    if(textureIterator != textureCache.end()) {
        return id;
    }

    spdlog::debug(
        "Creating text texture asset '{}' from text '{}'.",
        id.UID,
        text
    );

    TTF_Font *font = nullptr;

    try {
        const AssetID fontID = this->loadFont(resolvedFontPath, fontSize);
        font = &this->get<TTF_Font>(fontID);
    } catch(...) {
        this->forgetAsset(id);
        throw;
    }

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
        TTF_RenderUTF8_Blended(font, text.c_str(), colour),
        SDL_FreeSurface
    );

    if(surface == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to render text '") + text
            + "': " + TTF_GetError()
        );
    }

    TextureEntry entry;
    entry.size = SDL_Point{surface->w, surface->h};
    entry.type = AssetType::TextTexture;
    entry.texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if(entry.texture == nullptr) {
        this->forgetAsset(id);
        throw std::runtime_error(
            std::string("Failed to create text texture: ") + SDL_GetError()
        );
    }

    textureCache.emplace(key, std::move(entry));

    return id;
}

AssetRegistry::AssetID
AssetRegistry::loadYAML(const std::string &path, const std::string &context) {
    const std::string resolvedPath = this->resolvePath(path);
    const AssetID id = this->getOrCreateID(AssetType::YAML, resolvedPath);
    const auto YAMLIterator = this->YAMLDocuments.find(resolvedPath);

    if(YAMLIterator != this->YAMLDocuments.end()) {
        return id;
    }

    spdlog::info("Loading YAML asset '{}' for {}.", id.UID, context);

    try {
        auto [documentIterator, inserted] = this->YAMLDocuments.emplace(
            resolvedPath,
            YAML::LoadFile(resolvedPath)
        );
        (void)documentIterator;
        (void)inserted;

        spdlog::info("Loaded YAML asset '{}' for {}.", id.UID, context);

        return id;
    } catch(const YAML::Exception &exception) {
        this->forgetAsset(id);
        throw std::runtime_error(
            "Failed to load " + context + " '" + resolvedPath
            + "': " + exception.what()
        );
    }
}

} // namespace Engine
