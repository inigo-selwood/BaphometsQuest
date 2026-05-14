#include "sceneLoader.hpp"

#include "../parse/xml.hpp"

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

        setNodePropertyFromString(node, attributeName, attribute->Value());
    }
}

std::unique_ptr<Node> buildNodeSubtree(
    const tinyxml2::XMLElement &element,
    const std::string &nodeName
) {
    const std::string nodeType = element.Name();
    auto node = Node::create(nodeType);
    setNodeNameForLoading(*node, nodeName);

    applyNodeProperties(*node, element);

    for(const tinyxml2::XMLElement *childElement = element.FirstChildElement();
        childElement != nullptr;
        childElement = childElement->NextSiblingElement()) {
        const std::string childName = Parse::XML::requireAttribute(
            *childElement,
            NAME_ATTRIBUTE,
            "Node '" + std::string(childElement->Name()) + "'"
        );

        auto child = buildNodeSubtree(*childElement, childName);
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
        const std::string nodeName = Parse::XML::requireAttribute(
            *element,
            NAME_ATTRIBUTE,
            "Node '" + nodeType + "'"
        );

        parent.addChild(nodeName, buildNodeSubtree(*element, nodeName));
    }
}

} // namespace

void loadSceneFromXML(
    Node &scene,
    AssetRegistry &assets,
    const std::string &path
) {
    const AssetRegistry::AssetID sceneXMLID =
        assets.loadXML(path, "scene XML");
    const auto &sceneXML = assets.get<tinyxml2::XMLDocument>(sceneXMLID);

    const tinyxml2::XMLElement &root = Parse::XML::requireRootElement(
        sceneXML,
        SCENE_ROOT,
        "Scene XML '" + assets.resolvePath(path) + "'"
    );

    loadNodeChildren(scene, root);
    assets.unload(sceneXMLID);
}

} // namespace Engine
