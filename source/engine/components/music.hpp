#pragma once

#include "../assets/assetRegistry.hpp"
#include "../node/node.hpp"

#include <optional>
#include <string>

namespace Engine {

/** Scene node that plays an SDL_mixer music asset. */
class Music : public Node {
  public:
    Music();

    /** Register the Music XML node type. */
    static void registerType();

    /** Start or restart music playback. */
    void play();

    /** Stop current music playback. */
    void stop();

  private:
    void ensureLoaded();
    void onEnterTree() override;
    void onExitTree() override;
    void setVolume(int value);

    bool autoPlay = true;
    int fadeInMS = 0;
    int loops = -1;
    std::string path;
    bool stopOnExit = true;
    std::optional<AssetRegistry::AssetID> musicID;
    int volume = MIX_MAX_VOLUME;
};

} // namespace Engine
