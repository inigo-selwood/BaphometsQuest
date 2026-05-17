#pragma once

#include "../base.hpp"

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

    std::string describe() const override;

    const std::unique_ptr<tinyxml2::XMLDocument> document;
    const std::string path;
};

} // namespace Engine::Resource
