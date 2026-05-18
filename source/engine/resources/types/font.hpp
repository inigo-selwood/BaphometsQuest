#pragma once

#include "../base.hpp"

#include <SDL_ttf.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

class Font : public Engine::Resource::Base {
  private:
    struct FontDeleter {
        void operator()(TTF_Font *font) const {
            TTF_CloseFont(font);
        }
    };

    static std::unique_ptr<TTF_Font, FontDeleter>
    load(const std::string &path, int size);

  public:
    Font(const std::string &path, int size);

    static constexpr std::chrono::seconds TTL{120};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path, int size) {
        return std::make_unique<Font>(path, size);
    }

    static Engine::Resource::ID key(const std::string &path, int size);

    std::string describe() const override;

    const std::unique_ptr<TTF_Font, FontDeleter> handle;
    const std::string path;
    const int size = 0;
};

} // namespace Engine::Resource
