#include "scene_loader.hpp"

#include "../nodes/native/box.hpp"
#include "../nodes/native/camera.hpp"
#include "../nodes/native/canvas_layer.hpp"
#include "../nodes/native/image.hpp"
#include "../nodes/native/label.hpp"
#include "../nodes/native/map.hpp"
#include "../nodes/native/menu.hpp"
#include "../nodes/native/music.hpp"
#include "../nodes/native/sprite.hpp"
#include "../resources/types/xml.hpp"
#include "game.hpp"

#include <tinyxml2.h>

#include <exception>
#include <filesystem>
#include <string>
#include <vector>

namespace Engine {

SceneLoader::SceneLoader(Engine::Nodes::Base &parent)
    : parent(parent.shared_from_this()) {}

void SceneLoader::registerNativeNodes() {
    registerGlobalNode<Engine::Nodes::Box>("box");
    registerGlobalNode<Engine::Nodes::Camera>("camera");
    registerGlobalNode<Engine::Nodes::CanvasLayer>("canvas-layer");
    registerGlobalNode<Engine::Nodes::Image>("image");
    registerGlobalNode<Engine::Nodes::Label>("label");
    registerGlobalNode<Engine::Nodes::Map>("map");
    registerGlobalNode<Engine::Nodes::Menu>("menu");
    registerGlobalNode<Engine::Nodes::Music>("music");
    registerGlobalNode<Engine::Nodes::Sprite>("sprite");
}

void SceneLoader::load(const std::string &path) {
    std::shared_ptr<Engine::Nodes::Base> parent = this->getParent();
    std::vector<std::string> importStack;

    this->loadScene(*parent, path, importStack, true);
}

void SceneLoader::loadScene(
    Engine::Nodes::Base &parent,
    const std::string &path,
    std::vector<std::string> &importStack,
    bool assignRootName
) const {
    for(const std::string &importedPath : importStack) {
        if(importedPath == path) {
            throw std::runtime_error(
                "Scene XML import cycle detected for '" + path + "'"
            );
        }
    }

    importStack.push_back(path);
    Engine::Game &game = parent.getGame();
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

    const char *nameAttribute = root->Attribute("name");

    if(nameAttribute == nullptr || std::string{nameAttribute}.empty()) {
        throw std::runtime_error(
            "Scene XML '" + path + "' root element requires a name attribute"
        );
    }

    if(assignRootName) {
        parent.name = nameAttribute;
    }

    this->loadChildren(parent, *root, path, importStack);
    importStack.pop_back();
}

std::shared_ptr<Engine::Nodes::Base> SceneLoader::getParent() const {
    std::shared_ptr<Engine::Nodes::Base> parent = this->parent.lock();

    if(parent == nullptr) {
        throw std::runtime_error("Scene loader parent is no longer available");
    }

    return parent;
}

const SceneLoader::NodeCreator *SceneLoader::getNodeCreator(
    const std::unordered_map<std::string, NodeCreator> &localNodeCreators,
    const std::string &elementName
) {
    const auto localNodeCreator = localNodeCreators.find(elementName);

    if(localNodeCreator != localNodeCreators.end()) {
        return &localNodeCreator->second;
    }

    const auto &globalNodeCreators = getGlobalNodeCreators();
    const auto globalNodeCreator = globalNodeCreators.find(elementName);

    if(globalNodeCreator != globalNodeCreators.end()) {
        return &globalNodeCreator->second;
    }

    return nullptr;
}

std::unordered_map<std::string, SceneLoader::NodeCreator> &
SceneLoader::getGlobalNodeCreators() {
    static std::unordered_map<std::string, NodeCreator> nodeCreators;

    return nodeCreators;
}

void SceneLoader::loadChildren(
    Engine::Nodes::Base &parent,
    const tinyxml2::XMLElement &element,
    const std::string &path,
    std::vector<std::string> &importStack
) const {
    for(const tinyxml2::XMLElement *child = element.FirstChildElement();
        child != nullptr;
        child = child->NextSiblingElement()) {
        this->loadNode(parent, *child, path, importStack);
    }
}

void SceneLoader::loadNode(
    Engine::Nodes::Base &parent,
    const tinyxml2::XMLElement &element,
    const std::string &path,
    std::vector<std::string> &importStack
) const {
    const std::string elementName = element.Name();

    if(elementName == "import") {
        this->loadImport(parent, element, path, importStack);
        return;
    }

    const NodeCreator *nodeCreator = getNodeCreator(
        this->nodeCreators,
        elementName
    );

    if(nodeCreator == nullptr) {
        throw std::runtime_error(
            "Scene XML element '" + elementName + "' is not registered"
        );
    }

    const char *nameAttribute = element.Attribute("name");
    const std::string nodeName =
        nameAttribute == nullptr ? std::string{} : nameAttribute;
    std::shared_ptr<Engine::Nodes::Base> node = (*nodeCreator)();

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

    if(!node->loadXmlChildren(element)) {
        this->loadChildren(*node, element, path, importStack);
    }
}

void SceneLoader::loadImport(
    Engine::Nodes::Base &parent,
    const tinyxml2::XMLElement &element,
    const std::string &path,
    std::vector<std::string> &importStack
) const {
    const char *pathAttribute = element.Attribute("path");

    if(pathAttribute == nullptr || std::string{pathAttribute}.empty()) {
        throw std::runtime_error(
            "Scene XML import in '" + path + "' requires a path attribute"
        );
    }

    this->loadScene(
        parent,
        resolvePath(path, pathAttribute),
        importStack,
        false
    );
}

std::string SceneLoader::resolvePath(
    const std::string &path,
    const std::string &importPath
) {
    const std::filesystem::path requestedPath{importPath};

    if(requestedPath.is_absolute() || std::filesystem::exists(requestedPath)) {
        return requestedPath.lexically_normal().generic_string();
    }

    const std::filesystem::path importingPath{path};
    const std::filesystem::path relativePath =
        importingPath.parent_path() / requestedPath;

    return relativePath.lexically_normal().generic_string();
}

} // namespace Engine
