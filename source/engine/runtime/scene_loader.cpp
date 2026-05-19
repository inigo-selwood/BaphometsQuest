#include "scene_loader.hpp"

#include "../resources/types/xml.hpp"
#include "game.hpp"

#include <tinyxml2.h>

#include <exception>

namespace Engine {

SceneLoader::SceneLoader(Engine::Nodes::Base &parent)
    : parent(parent.shared_from_this()) {}

void SceneLoader::load(const std::string &path) {
    std::shared_ptr<Engine::Nodes::Base> parent = this->getParent();
    Engine::Game &game = parent->getGame();
    const Engine::Resource::ID xmlID =
        game.resources.load<Engine::Resource::XML>(path);
    const Engine::Resource::XML &xml =
        game.resources.get<Engine::Resource::XML>(xmlID);
    const tinyxml2::XMLElement *root = xml.document->RootElement();

    if(root == nullptr) {
        throw std::runtime_error("Scene XML '" + path + "' is empty");
    }

    if(std::string(root->Name()) != "scene") {
        throw std::runtime_error(
            "Scene XML '" + path + "' root element must be <scene>"
        );
    }

    this->loadChildren(*parent, *root);
}

std::shared_ptr<Engine::Nodes::Base> SceneLoader::getParent() const {
    std::shared_ptr<Engine::Nodes::Base> parent = this->parent.lock();

    if(parent == nullptr) {
        throw std::runtime_error("Scene loader parent is no longer available");
    }

    return parent;
}

void SceneLoader::loadChildren(
    Engine::Nodes::Base &parent,
    const tinyxml2::XMLElement &element
) const {
    for(const tinyxml2::XMLElement *child = element.FirstChildElement();
        child != nullptr;
        child = child->NextSiblingElement()) {
        this->loadNode(parent, *child);
    }
}

void SceneLoader::loadNode(
    Engine::Nodes::Base &parent,
    const tinyxml2::XMLElement &element
) const {
    const std::string elementName = element.Name();
    const auto nodeCreator = this->nodeCreators.find(elementName);

    if(nodeCreator == this->nodeCreators.end()) {
        throw std::runtime_error(
            "Scene XML element '" + elementName + "' is not registered"
        );
    }

    const char *nameAttribute = element.Attribute("name");
    const std::string nodeName =
        nameAttribute == nullptr ? std::string{} : nameAttribute;
    std::shared_ptr<Engine::Nodes::Base> node = nodeCreator->second();

    if(node == nullptr) {
        throw std::runtime_error(
            "Scene XML element '" + elementName + "' creator returned null"
        );
    }

    parent.addChild(nodeName, node);

    for(const tinyxml2::XMLAttribute *attribute = element.FirstAttribute();
        attribute != nullptr;
        attribute = attribute->Next()) {
        const std::string attributeName = attribute->Name();

        if(attributeName == "name") {
            continue;
        }

        try {
            node->setPropertyFromText(attributeName, attribute->Value());
        } catch(const std::exception &exception) {
            throw std::runtime_error(
                "Scene XML element '" + elementName + "' property '"
                + attributeName + "' with value '" + attribute->Value()
                + "' failed: " + exception.what()
            );
        }
    }

    this->loadChildren(*node, element);
}

} // namespace Engine
