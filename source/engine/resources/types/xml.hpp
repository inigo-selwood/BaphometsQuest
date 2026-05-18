#pragma once

#include "../base.hpp"

#include <chrono>
#include <memory>
#include <string>

#include <tinyxml2.h>

namespace Engine::Resource {

class XML : public Engine::Resource::Base {
  private:
    static std::unique_ptr<tinyxml2::XMLDocument>
    load(const std::string &path);

  public:
    explicit XML(const std::string &path);

    static constexpr std::chrono::seconds TTL{300};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<XML>(path);
    }

    static Engine::Resource::Key key(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<tinyxml2::XMLDocument> document;
    const std::string path;
};

} // namespace Engine::Resource
