#pragma once

#include <tinyxml2.h>

#include <string>

namespace Engine {

namespace Parse {

namespace XML {

/** Return a required XML attribute value. */
std::string requireAttribute(
    const tinyxml2::XMLElement &element,
    const std::string &name,
    const std::string &context
);

/** Return the required document root element with the expected name. */
const tinyxml2::XMLElement &requireRootElement(
    const tinyxml2::XMLDocument &document,
    const std::string &name,
    const std::string &context
);

} // namespace XML

} // namespace Parse

} // namespace Engine
