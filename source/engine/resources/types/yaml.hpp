#pragma once

#include "../base.hpp"

#include <memory>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

class YAML : public Engine::Resource::Base {
  public:
    explicit YAML(const std::string &path);

    static std::string key(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<::YAML::Node> node;
    const std::string path;
};

} // namespace Engine::Resource
