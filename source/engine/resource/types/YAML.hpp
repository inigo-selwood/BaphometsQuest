#pragma once

#include "../Base.hpp"

#include <memory>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

class YAML : public Engine::Resource::Base {
  public:
    explicit YAML(const std::string &path);

    ::YAML::Node &getNode() const;
    std::string describe() const override;

  private:
    std::unique_ptr<::YAML::Node> node;
    std::string path;
};

} // namespace Engine::Resource
