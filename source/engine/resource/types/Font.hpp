#pragma once

#include "../Base.hpp"

#include <SDL_ttf.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class Font : public Engine::Resource::Base {
  public:
    Font(const std::string &path, int size);

    TTF_Font *getHandle() const;
    std::string describe() const override;

  private:
    struct FontDeleter {
        void operator()(TTF_Font *font) const;
    };

    std::unique_ptr<TTF_Font, FontDeleter> font;
    std::string path;
    int size = 0;
};

} // namespace Engine::Resource
