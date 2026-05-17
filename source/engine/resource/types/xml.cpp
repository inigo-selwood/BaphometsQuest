#include "xml.hpp"

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

XML::XML(const std::string &path) : document(load(path)), path(path) {}
std::string XML::describe() const {
    ::YAML::Node name;
    name["type"] = "XML";
    name["path"] = this->path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
