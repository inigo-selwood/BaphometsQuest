#pragma once

#include "../base.hpp"

#include <chrono>
#include <memory>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

class YAML : public Engine::Resource::Base {
  public:
    explicit YAML(const std::string &path);

    static constexpr std::chrono::seconds TTL{300};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<YAML>(path);
    }

    static Engine::Resource::ID key(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<::YAML::Node> node;
    const std::string path;
};

} // namespace Engine::Resource
