#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
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
    enum class AssetType {
        Font,
        ImageTexture,
        TextTexture,
        Music,
        SoundEffect,
        XML,
        YAML,
    };

    struct AssetID {
        std::uint64_t UID = 0;
    };

    struct AssetGroupID {
        std::uint64_t UID = 0;
    };

    /** Destroy all cached assets and parsed documents. */
    void clear();

    /** Stop assigning new assets to an asset group. */
    void clearActiveGroup();

    /** Create a group that can own newly loaded assets. */
    AssetGroupID createGroup(const std::string &name);

    /** Return a cached asset by ID. */
    template <typename Asset> Asset &get(const AssetID &id) {
        if constexpr(std::is_same_v<Asset, TTF_Font>) {
            return *this->getFont(id);
        } else if constexpr(std::is_same_v<Asset, SDL_Texture>) {
            return *this->getTexture(id);
        } else if constexpr(std::is_same_v<Asset, Mix_Music>) {
            return *this->getMusic(id);
        } else if constexpr(std::is_same_v<Asset, Mix_Chunk>) {
            return *this->getSoundEffect(id);
        } else if constexpr(std::is_same_v<Asset, tinyxml2::XMLDocument>) {
            return *this->getXML(id);
        } else if constexpr(std::is_same_v<Asset, YAML::Node>) {
            return *this->getYAML(id);
        } else {
            throw std::runtime_error("Unsupported asset access type.");
        }
    }

    /** Load or fetch a cached TTF font. */
    AssetID loadFont(const std::string &path, int size);

    /** Load or fetch a cached image texture. */
    AssetID loadImageTexture(SDL_Renderer *renderer, const std::string &path);

    /** Load or fetch cached music. */
    AssetID loadMusic(const std::string &path);

    /** Load or fetch a cached sound effect. */
    AssetID loadSoundEffect(const std::string &path);

    /** Load or fetch a cached XML document. */
    AssetID loadXML(const std::string &path, const std::string &context);

    /** Load or fetch a cached texture generated from rendered text. */
    AssetID loadTextTexture(
        SDL_Renderer *renderer,
        const std::string &fontPath,
        int fontSize,
        SDL_Color colour,
        const std::string &text
    );

    /** Load or fetch a cached YAML document. */
    AssetID loadYAML(const std::string &path, const std::string &context);

    /** Resolve a resource path against bundled resources when possible. */
    std::string resolvePath(const std::string &path) const;

    /** Assign newly loaded assets to a group until the active group changes.
     */
    void setActiveGroup(AssetGroupID group);

    /** Return the pixel size for a loaded texture asset. */
    SDL_Point getTextureSize(const AssetID &id) const;

    /** Destroy one cached asset. */
    void unload(const AssetID &id);

    /** Destroy all cached assets. */
    void unloadAll();

    /** Destroy all cached assets of one type. */
    void unloadAll(AssetType type);

    /** Destroy all cached assets owned by a group. */
    void unloadGroup(AssetGroupID group);

  private:
    struct FontDeleter {
        void operator()(TTF_Font *font) const {
            TTF_CloseFont(font);
        }
    };

    struct AssetRecord {
        AssetType type;
        std::string key;
        AssetGroupID group;
    };

    struct MusicDeleter {
        void operator()(Mix_Music *music) const {
            Mix_FreeMusic(music);
        }
    };

    struct SoundEffectDeleter {
        void operator()(Mix_Chunk *soundEffect) const {
            Mix_FreeChunk(soundEffect);
        }
    };

    struct TextureDeleter {
        void operator()(SDL_Texture *texture) const {
            SDL_DestroyTexture(texture);
        }
    };

    struct TextureEntry {
        std::unique_ptr<SDL_Texture, TextureDeleter> texture;
        SDL_Point size{0, 0};
        AssetType type = AssetType::ImageTexture;
    };

    static std::string makeFontKey(const std::string &path, int size);
    static std::string makeLookupKey(AssetType type, const std::string &key);
    static std::string makeTextTextureKey(
        const std::string &fontPath,
        int fontSize,
        SDL_Color colour,
        const std::string &text
    );
    static std::string typeName(AssetType type);

    void forgetAsset(const AssetID &id);
    const AssetRecord &getRecord(const AssetID &id) const;
    AssetID getOrCreateID(AssetType type, const std::string &key);
    TTF_Font *getFont(const AssetID &id);
    Mix_Music *getMusic(const AssetID &id);
    Mix_Chunk *getSoundEffect(const AssetID &id);
    SDL_Texture *getTexture(const AssetID &id);
    tinyxml2::XMLDocument *getXML(const AssetID &id);
    YAML::Node *getYAML(const AssetID &id);
    static bool isTextureType(AssetType type);

    std::unordered_map<std::uint64_t, AssetRecord> assets;
    std::unordered_map<std::uint64_t, std::string> assetGroupNames;
    std::unordered_map<std::string, std::uint64_t> assetIDs;
    std::unordered_map<std::string, std::unique_ptr<TTF_Font, FontDeleter>>
        fonts;
    std::unordered_map<std::string, std::unique_ptr<Mix_Music, MusicDeleter>>
        music;
    std::unordered_map<
        std::string,
        std::unique_ptr<Mix_Chunk, SoundEffectDeleter>>
        soundEffects;
    std::unordered_map<std::string, TextureEntry> textures;
    std::unordered_map<std::string, std::unique_ptr<tinyxml2::XMLDocument>>
        XMLDocuments;
    std::unordered_map<std::string, YAML::Node> YAMLDocuments;
    std::optional<AssetGroupID> activeAssetGroup;
    std::uint64_t nextAssetGroupUID = 1;
    std::uint64_t nextAssetUID = 1;
};

} // namespace Engine
