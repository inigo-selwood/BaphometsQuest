#pragma once

#include "../base.hpp"

#include <SDL_ttf.h>

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

    static std::string key(const std::string &path, int size);

    std::string describe() const override;

    const std::unique_ptr<TTF_Font, FontDeleter> handle;
    const std::string path;
    const int size = 0;
};

} // namespace Engine::Resource
