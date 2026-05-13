#include "sceneLoader.hpp"

#include <tinyxml2.h>

#include <string>

namespace Engine {

namespace {

const char *SCENE_ROOT = "Scene";
const char *NAME_ATTRIBUTE = "name";

void applyNodeProperties(Node &node, const tinyxml2::XMLElement &element) {
    for(const tinyxml2::XMLAttribute *attribute = element.FirstAttribute();
        attribute != nullptr;
        attribute = attribute->Next()) {
        const std::string attributeName = attribute->Name();

        if(attributeName == NAME_ATTRIBUTE) {
            continue;
        }

        node.setProperty(attributeName, attribute->Value());
    }
}

std::unique_ptr<Node> buildNodeSubtree(const tinyxml2::XMLElement &element) {
    const std::string nodeType = element.Name();
    auto node = Node::create(nodeType);

    applyNodeProperties(*node, element);

    for(const tinyxml2::XMLElement *childElement = element.FirstChildElement();
        childElement != nullptr;
        childElement = childElement->NextSiblingElement()) {
        const char *childName = childElement->Attribute(NAME_ATTRIBUTE);

        if(childName == nullptr) {
            throw std::runtime_error(
                "Node '" + std::string(childElement->Name())
                + "' is missing required name."
            );
        }

        auto child = buildNodeSubtree(*childElement);
        node->addChild(childName, std::move(child));
    }

    return node;
}

void loadNodeChildren(
    Node &parent,
    const tinyxml2::XMLElement &parentElement
) {
    for(const tinyxml2::XMLElement *element =
            parentElement.FirstChildElement();
        element != nullptr;
        element = element->NextSiblingElement()) {
        const std::string nodeType = element->Name();
        const char *nodeName = element->Attribute(NAME_ATTRIBUTE);

        if(nodeName == nullptr) {
            throw std::runtime_error(
                "Node '" + nodeType + "' is missing required name."
            );
        }

        parent.addChild(nodeName, buildNodeSubtree(*element));
    }
}

} // namespace

void loadSceneFromXML(
    Scene &scene,
    AssetRegistry &assets,
    const std::string &path
) {
    const AssetRegistry::AssetID sceneXMLID =
        assets.loadXML(path, "scene XML");
    const auto &sceneXML = assets.get<tinyxml2::XMLDocument>(sceneXMLID);

    const tinyxml2::XMLElement *root = sceneXML.RootElement();

    if(root == nullptr || std::string(root->Name()) != SCENE_ROOT) {
        throw std::runtime_error(
            "Scene XML '" + assets.resolvePath(path)
            + "' must have a Scene root element."
        );
    }

    loadNodeChildren(scene, *root);
    assets.unload(sceneXMLID);
}

} // namespace Engine
