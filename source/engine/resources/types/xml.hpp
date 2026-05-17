#pragma once

#include "../base.hpp"

#include <memory>
#include <string>

#include <tinyxml2.h>

namespace Engine::Resource {

class XML : public Engine::Resource::Base {
  public:
    explicit XML(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<tinyxml2::XMLDocument> Document;
    const std::string Path;
};

} // namespace Engine::Resource
