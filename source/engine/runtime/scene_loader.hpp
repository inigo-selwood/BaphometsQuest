#pragma once

#include "../nodes/base.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace tinyxml2 {
class XMLElement;
class XMLDocument;
} // namespace tinyxml2

namespace Engine {

/** Populate a scene node tree from declarative XML */
class SceneLoader {
  private:
    using NodeCreator = std::function<std::shared_ptr<Engine::Nodes::Base>()>;

  public:
    explicit SceneLoader(Engine::Nodes::Base &parent);

    /** Register engine-native XML node types */
    static void registerNativeNodes();

    /** Register a default-constructible node for an XML element name */
    template <typename NodeType>
    void registerNode(const std::string &elementName) {
        static_assert(
            std::is_base_of_v<Engine::Nodes::Base, NodeType>,
            "NodeType must inherit from Engine::Nodes::Base."
        );

        if(elementName.empty()) {
            throw std::runtime_error(
                "Scene loader element name must not be empty"
            );
        }

        if(getGlobalNodeCreators().contains(elementName)) {
            throw std::runtime_error(
                "Scene loader element '" + elementName
                + "' is already globally registered"
            );
        }

        if(this->nodeCreators.contains(elementName)) {
            throw std::runtime_error(
                "Scene loader element '" + elementName
                + "' is already registered"
            );
        }

        this->nodeCreators.emplace(elementName, []() {
            return std::make_shared<NodeType>();
        });
    }

    /** Load XML and append its nodes to the parent node */
    void load(const std::string &path);

  private:
    template <typename NodeType>
    static void registerGlobalNode(const std::string &elementName) {
        static_assert(
            std::is_base_of_v<Engine::Nodes::Base, NodeType>,
            "NodeType must inherit from Engine::Nodes::Base."
        );

        if(elementName.empty()) {
            throw std::runtime_error(
                "Scene loader element name must not be empty"
            );
        }

        auto &nodeCreators = getGlobalNodeCreators();

        if(nodeCreators.contains(elementName)) {
            return;
        }

        nodeCreators.emplace(elementName, []() {
            return std::make_shared<NodeType>();
        });
    }

    static const NodeCreator *getNodeCreator(
        const std::unordered_map<std::string, NodeCreator> &localNodeCreators,
        const std::string &elementName
    );

    static std::unordered_map<std::string, NodeCreator> &
    getGlobalNodeCreators();

    void loadScene(
        Engine::Nodes::Base &parent,
        const std::string &path,
        std::vector<std::string> &importStack,
        bool assignRootName
    ) const;

    void loadChildren(
        Engine::Nodes::Base &parent,
        const tinyxml2::XMLElement &element,
        const std::string &path,
        std::vector<std::string> &importStack
    ) const;

    void loadNode(
        Engine::Nodes::Base &parent,
        const tinyxml2::XMLElement &element,
        const std::string &path,
        std::vector<std::string> &importStack
    ) const;

    void loadImport(
        Engine::Nodes::Base &parent,
        const tinyxml2::XMLElement &element,
        const std::string &path,
        std::vector<std::string> &importStack
    ) const;

    const tinyxml2::XMLElement &loadRoot(const std::string &path) const;

    static void
    validateRoot(const tinyxml2::XMLElement &root, const std::string &path);

    tinyxml2::XMLElement *expandImports(
        const tinyxml2::XMLElement &element,
        const std::string &path,
        std::vector<std::string> &importStack,
        tinyxml2::XMLDocument &document
    ) const;

    static std::string
    resolvePath(const std::string &path, const std::string &importPath);

    std::shared_ptr<Engine::Nodes::Base> getParent() const;

    std::weak_ptr<Engine::Nodes::Base> parent;
    std::unordered_map<std::string, NodeCreator> nodeCreators;
};

} // namespace Engine
