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
    loadFont(const std::string &path, int size);

  public:
    Font(const std::string &path, int size);

    std::string describe() const override;

    const std::unique_ptr<TTF_Font, FontDeleter> Handle;
    const std::string Path;
    const int Size = 0;
};

} // namespace Engine::Resource
