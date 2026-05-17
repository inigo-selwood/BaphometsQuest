#include "XML.hpp"

#include <stdexcept>

namespace Engine::Resource {

XML::XML(const std::string &path)
    : document(std::make_unique<tinyxml2::XMLDocument>()), path(path) {
    const tinyxml2::XMLError error = this->document->LoadFile(path.c_str());

    if(error != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Failed to load XML '" + path + "': " + this->document->ErrorStr()
        );
    }
}

tinyxml2::XMLDocument &XML::getDocument() const {
    return *this->document;
}

std::string XML::describe() const {
    ::YAML::Node name;
    name["type"] = "XML";
    name["path"] = this->path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
