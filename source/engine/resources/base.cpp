#include "base.hpp"

#include <atomic>
#include <functional>
#include <iomanip>
#include <sstream>

namespace Engine::Resource {

namespace {

std::atomic<std::uint32_t> nextID = 1;

} // namespace

Base::Base() : ID(generateID()) {}

std::string Base::formatDescription(const ::YAML::Node &node) {
    ::YAML::Emitter emitter;

    emitter << ::YAML::Block;
    emitter << node;

    return emitter.c_str();
}

Engine::Resource::Key Base::hashKey(const std::string &value) {
    return static_cast<Engine::Resource::Key>(std::hash<std::string>{}(value));
}

std::string Base::generateID() {
    std::ostringstream stream;

    stream << std::hex << std::nouppercase << std::setfill('0') << std::setw(4)
           << nextID++;

    return stream.str();
}

} // namespace Engine::Resource
