#pragma once

#include "../base.hpp"

#include <memory>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

class YAML : public Engine::Resource::Base {
  public:
    explicit YAML(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<::YAML::Node> Node;
    const std::string Path;
};

} // namespace Engine::Resource
