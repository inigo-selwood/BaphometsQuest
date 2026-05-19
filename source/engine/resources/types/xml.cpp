#include "xml.hpp"

#include "../../utils/format.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<tinyxml2::XMLDocument> XML::load(const std::string &path) {
    auto document = std::make_unique<tinyxml2::XMLDocument>();
    const tinyxml2::XMLError error = document->LoadFile(path.c_str());

    if(error != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Failed to load XML '" + path + "': " + document->ErrorStr()
        );
    }

    return document;
}

XML::XML(const std::string &path)
    : Base("xml"), document(load(path)), path(path) {}

Engine::Resource::ID XML::key(const std::string &path) {
    return hashKey("XML:" + path);
}

std::string XML::describe() const {
    ::YAML::Node name;
    name["type"] = "XML";
    name["path"] = Engine::Format::path(this->path);

    return this->formatDescription(name);
}

} // namespace Engine::Resource
