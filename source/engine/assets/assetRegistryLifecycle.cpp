#include "assetRegistry.hpp"

#include <stdexcept>

namespace Engine {

void AssetRegistry::unload(const AssetID &id) {
    const auto assetIterator = this->assets.find(id.UID);

    if(id.UID == 0 || assetIterator == this->assets.end()) {
        spdlog::debug("Cached asset '{}' was already unloaded.", id.UID);
        return;
    }

    const AssetType type = assetIterator->second.type;
    const std::string key = assetIterator->second.key;
    spdlog::debug(
        "Unloading cached {} asset '{}'.",
        AssetRegistry::typeName(type),
        id.UID
    );

    std::size_t removed = 0;

    switch(type) {
    case AssetType::Font:
        removed = this->fonts.erase(key);
        break;
    case AssetType::ImageTexture:
    case AssetType::TextTexture:
        removed = this->textures.erase(key);
        break;
    case AssetType::Music:
        removed = this->music.erase(key);
        break;
    case AssetType::SoundEffect:
        removed = this->soundEffects.erase(key);
        break;
    case AssetType::XML:
        removed = this->XMLDocuments.erase(key);
        break;
    case AssetType::YAML:
        removed = this->YAMLDocuments.erase(key);
        break;
    }

    if(removed == 0) {
        spdlog::debug(
            "Cached {} asset '{}' was already unloaded.",
            AssetRegistry::typeName(type),
            id.UID
        );
        this->forgetAsset(id);
        return;
    }

    this->forgetAsset(id);
}

void AssetRegistry::unloadAll() {
    for(const auto &[UID, asset] : this->assets) {
        spdlog::debug(
            "Unloading cached {} asset '{}'.",
            AssetRegistry::typeName(asset.type),
            UID
        );
    }

    this->assetIDs.clear();
    this->assets.clear();
    this->fonts.clear();
    this->music.clear();
    this->soundEffects.clear();
    this->textures.clear();
    this->XMLDocuments.clear();
    this->YAMLDocuments.clear();
}

void AssetRegistry::unloadAll(AssetType type) {
    switch(type) {
    case AssetType::Font:
        for(auto iterator = this->assets.begin();
            iterator != this->assets.end();) {
            if(iterator->second.type != type) {
                iterator++;
                continue;
            }

            spdlog::debug(
                "Unloading cached font asset '{}'.",
                iterator->first
            );
            this->assetIDs.erase(
                AssetRegistry::makeLookupKey(
                    iterator->second.type,
                    iterator->second.key
                )
            );
            iterator = this->assets.erase(iterator);
        }

        this->fonts.clear();
        break;
    case AssetType::ImageTexture:
    case AssetType::TextTexture:
        for(auto iterator = this->textures.begin();
            iterator != this->textures.end();) {
            const bool matchingImage = type == AssetType::ImageTexture
                && iterator->second.type == AssetType::ImageTexture;
            const bool matchingText = type == AssetType::TextTexture
                && iterator->second.type == AssetType::TextTexture;

            if(matchingImage || matchingText) {
                const std::string key = iterator->first;
                std::uint64_t UID = 0;
                const auto assetIDIterator = this->assetIDs.find(
                    AssetRegistry::makeLookupKey(type, key)
                );

                if(assetIDIterator != this->assetIDs.end()) {
                    UID = assetIDIterator->second;
                }

                spdlog::debug(
                    "Unloading cached {} asset '{}'.",
                    AssetRegistry::typeName(type),
                    UID
                );
                this->assetIDs.erase(AssetRegistry::makeLookupKey(type, key));

                if(UID != 0) {
                    this->assets.erase(UID);
                }

                iterator = this->textures.erase(iterator);
                continue;
            }

            iterator++;
        }

        break;
    case AssetType::Music:
        for(auto iterator = this->assets.begin();
            iterator != this->assets.end();) {
            if(iterator->second.type != type) {
                iterator++;
                continue;
            }

            spdlog::debug(
                "Unloading cached music asset '{}'.",
                iterator->first
            );
            this->assetIDs.erase(
                AssetRegistry::makeLookupKey(
                    iterator->second.type,
                    iterator->second.key
                )
            );
            iterator = this->assets.erase(iterator);
        }

        this->music.clear();
        break;
    case AssetType::SoundEffect:
        for(auto iterator = this->assets.begin();
            iterator != this->assets.end();) {
            if(iterator->second.type != type) {
                iterator++;
                continue;
            }

            spdlog::debug(
                "Unloading cached sound effect asset '{}'.",
                iterator->first
            );
            this->assetIDs.erase(
                AssetRegistry::makeLookupKey(
                    iterator->second.type,
                    iterator->second.key
                )
            );
            iterator = this->assets.erase(iterator);
        }

        this->soundEffects.clear();
        break;
    case AssetType::XML:
        for(auto iterator = this->assets.begin();
            iterator != this->assets.end();) {
            if(iterator->second.type != type) {
                iterator++;
                continue;
            }

            spdlog::debug("Unloading cached XML asset '{}'.", iterator->first);
            this->assetIDs.erase(
                AssetRegistry::makeLookupKey(
                    iterator->second.type,
                    iterator->second.key
                )
            );
            iterator = this->assets.erase(iterator);
        }

        this->XMLDocuments.clear();
        break;
    case AssetType::YAML:
        for(auto iterator = this->assets.begin();
            iterator != this->assets.end();) {
            if(iterator->second.type != type) {
                iterator++;
                continue;
            }

            spdlog::debug(
                "Unloading cached YAML asset '{}'.",
                iterator->first
            );
            this->assetIDs.erase(
                AssetRegistry::makeLookupKey(
                    iterator->second.type,
                    iterator->second.key
                )
            );
            iterator = this->assets.erase(iterator);
        }

        this->YAMLDocuments.clear();
        break;
    }
}

TTF_Font *AssetRegistry::getFont(const AssetID &id) {
    const AssetRecord &record = this->getRecord(id);

    if(record.type != AssetType::Font) {
        throw std::runtime_error("AssetID type is not font.");
    }

    const auto iterator = this->fonts.find(record.key);

    if(iterator == this->fonts.end()) {
        throw std::runtime_error(
            "Font asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return iterator->second.get();
}

Mix_Music *AssetRegistry::getMusic(const AssetID &id) {
    const AssetRecord &record = this->getRecord(id);

    if(record.type != AssetType::Music) {
        throw std::runtime_error("AssetID type is not music.");
    }

    const auto iterator = this->music.find(record.key);

    if(iterator == this->music.end()) {
        throw std::runtime_error(
            "Music asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return iterator->second.get();
}

Mix_Chunk *AssetRegistry::getSoundEffect(const AssetID &id) {
    const AssetRecord &record = this->getRecord(id);

    if(record.type != AssetType::SoundEffect) {
        throw std::runtime_error("AssetID type is not sound effect.");
    }

    const auto iterator = this->soundEffects.find(record.key);

    if(iterator == this->soundEffects.end()) {
        throw std::runtime_error(
            "Sound effect asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return iterator->second.get();
}

SDL_Texture *AssetRegistry::getTexture(const AssetID &id) {
    const AssetRecord &record = this->getRecord(id);

    if(!AssetRegistry::isTextureType(record.type)) {
        throw std::runtime_error("AssetID type is not texture.");
    }

    const auto iterator = this->textures.find(record.key);

    if(iterator == this->textures.end()) {
        throw std::runtime_error(
            "Texture asset is not loaded: " + std::to_string(id.UID)
        );
    }

    if(iterator->second.type != record.type) {
        throw std::runtime_error("Texture asset type does not match AssetID.");
    }

    return iterator->second.texture.get();
}

tinyxml2::XMLDocument *AssetRegistry::getXML(const AssetID &id) {
    const AssetRecord &record = this->getRecord(id);

    if(record.type != AssetType::XML) {
        throw std::runtime_error("AssetID type is not XML.");
    }

    const auto iterator = this->XMLDocuments.find(record.key);

    if(iterator == this->XMLDocuments.end()) {
        throw std::runtime_error(
            "XML asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return iterator->second.get();
}

YAML::Node *AssetRegistry::getYAML(const AssetID &id) {
    const AssetRecord &record = this->getRecord(id);

    if(record.type != AssetType::YAML) {
        throw std::runtime_error("AssetID type is not YAML.");
    }

    const auto iterator = this->YAMLDocuments.find(record.key);

    if(iterator == this->YAMLDocuments.end()) {
        throw std::runtime_error(
            "YAML asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return &iterator->second;
}

SDL_Point AssetRegistry::getTextureSize(const AssetID &id) const {
    const AssetRecord &record = this->getRecord(id);

    if(!AssetRegistry::isTextureType(record.type)) {
        throw std::runtime_error("AssetID type is not texture.");
    }

    const auto iterator = this->textures.find(record.key);

    if(iterator == this->textures.end()) {
        throw std::runtime_error(
            "Texture asset is not loaded: " + std::to_string(id.UID)
        );
    }

    return iterator->second.size;
}

} // namespace Engine
