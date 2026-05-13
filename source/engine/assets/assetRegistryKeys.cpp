#include "assetRegistry.hpp"

#include <SDL.h>

#include <filesystem>
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

std::string
AssetRegistry::makeLookupKey(AssetType type, const std::string &key) {
    return AssetRegistry::typeName(type) + ":" + key;
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

std::string AssetRegistry::typeName(AssetType type) {
    switch(type) {
    case AssetType::Font:
        return "font";
    case AssetType::ImageTexture:
        return "image texture";
    case AssetType::TextTexture:
        return "text texture";
    case AssetType::Music:
        return "music";
    case AssetType::SoundEffect:
        return "sound effect";
    case AssetType::XML:
        return "XML";
    case AssetType::YAML:
        return "YAML";
    }

    return "unknown";
}

void AssetRegistry::forgetAsset(const AssetID &id) {
    const auto assetIterator = this->assets.find(id.UID);

    if(assetIterator == this->assets.end()) {
        return;
    }

    this->assetIDs.erase(
        AssetRegistry::makeLookupKey(
            assetIterator->second.type,
            assetIterator->second.key
        )
    );
    this->assets.erase(assetIterator);
}

const AssetRegistry::AssetRecord &
AssetRegistry::getRecord(const AssetID &id) const {
    const auto iterator = this->assets.find(id.UID);

    if(id.UID == 0 || iterator == this->assets.end()) {
        throw std::runtime_error(
            "Asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return iterator->second;
}

AssetRegistry::AssetID
AssetRegistry::getOrCreateID(AssetType type, const std::string &key) {
    const std::string lookupKey = AssetRegistry::makeLookupKey(type, key);
    const auto existingIterator = this->assetIDs.find(lookupKey);

    if(existingIterator != this->assetIDs.end()) {
        return AssetID{existingIterator->second};
    }

    const AssetID id{this->nextAssetUID++};
    this->assetIDs.emplace(lookupKey, id.UID);
    this->assets.emplace(id.UID, AssetRecord{type, key});

    return id;
}

bool AssetRegistry::isTextureType(AssetType type) {
    return type == AssetType::ImageTexture || type == AssetType::TextTexture;
}

} // namespace Engine
