#include "xml.hpp"

#include <stdexcept>

namespace Engine {

namespace Parse {

namespace XML {

std::string requireAttribute(
    const tinyxml2::XMLElement &element,
    const std::string &name,
    const std::string &context
) {
    const char *value = element.Attribute(name.c_str());

    if(value == nullptr) {
        throw std::runtime_error(
            context + " is missing required attribute '" + name + "'."
        );
    }

    return value;
}

const tinyxml2::XMLElement &requireRootElement(
    const tinyxml2::XMLDocument &document,
    const std::string &name,
    const std::string &context
) {
    const tinyxml2::XMLElement *root = document.RootElement();

    if(root == nullptr || root->Name() == nullptr || root->Name() != name) {
        throw std::runtime_error(
            context + " must have a <" + name + "> root element."
        );
    }

    return *root;
}

} // namespace XML

} // namespace Parse

} // namespace Engine
