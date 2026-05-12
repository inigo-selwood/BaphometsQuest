#include "sceneLoader.hpp"

#include "game.hpp"

#include <tinyxml2.h>

#include <string>

namespace {

const char *SCENE_ROOT = "Scene";
const char *NAME_ATTRIBUTE = "name";

void loadNodeChildren(
        Node &parent, const tinyxml2::XMLElement &parentElement) {
    for (const tinyxml2::XMLElement *element =
                    parentElement.FirstChildElement();
            element != nullptr;
            element = element->NextSiblingElement()) {
        const std::string nodeType = element->Name();
        const char *nodeName = element->Attribute(NAME_ATTRIBUTE);

        if (nodeName == nullptr) {
            throw std::runtime_error(
                    "Node '" + nodeType + "' is missing required name.");
        }

        auto node = Node::create(nodeType);

        for (const tinyxml2::XMLAttribute *attribute =
                        element->FirstAttribute();
                attribute != nullptr;
                attribute = attribute->Next()) {
            const std::string attributeName = attribute->Name();

            if (attributeName == NAME_ATTRIBUTE) {
                continue;
            }

            node->setProperty(attributeName, attribute->Value());
        }

        auto &child = parent.addChild(nodeName, std::move(node));
        loadNodeChildren(child, *element);
    }
}

} // namespace

void loadSceneFromXML(Scene &scene, const std::string &path) {
    spdlog::info("Loading scene XML '{}'.", path);

    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult = document.LoadFile(path.c_str());

    if (loadResult != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load scene XML '" + path
                + "': " + document.ErrorStr());
    }

    const tinyxml2::XMLElement *root = document.RootElement();

    if (root == nullptr || std::string(root->Name()) != SCENE_ROOT) {
        throw std::runtime_error(
                "Scene XML '" + path + "' must have a Scene root element.");
    }

    loadNodeChildren(scene, *root);

    spdlog::info("Loaded scene XML '{}'.", path);
}
