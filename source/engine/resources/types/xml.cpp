#include "xml.hpp"

#include <stdexcept>

namespace Engine::Resource {

namespace {

std::unique_ptr<tinyxml2::XMLDocument> loadDocument(const std::string &path) {
    auto document = std::make_unique<tinyxml2::XMLDocument>();
    const tinyxml2::XMLError error = document->LoadFile(path.c_str());

    if(error != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Failed to load XML '" + path + "': " + document->ErrorStr()
        );
    }

    return document;
}

} // namespace

XML::XML(const std::string &path) : Document(loadDocument(path)), Path(path) {}
std::string XML::describe() const {
    ::YAML::Node name;
    name["type"] = "XML";
    name["path"] = this->Path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
