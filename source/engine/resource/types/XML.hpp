#pragma once

#include "../Base.hpp"

#include <memory>
#include <string>

#include <tinyxml2.h>

namespace Engine::Resource {

class XML : public Engine::Resource::Base {
  public:
    explicit XML(const std::string &path);

    tinyxml2::XMLDocument &getDocument() const;
    std::string describe() const override;

  private:
    std::unique_ptr<tinyxml2::XMLDocument> document;
    std::string path;
};

} // namespace Engine::Resource
