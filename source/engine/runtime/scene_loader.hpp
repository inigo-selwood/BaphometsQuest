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
}

namespace Engine {

/** Populate a scene node tree from declarative XML */
class SceneLoader {
  public:
    explicit SceneLoader(Engine::Nodes::Base &parent);

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
    using NodeCreator = std::function<std::shared_ptr<Engine::Nodes::Base>()>;

    void loadScene(
        Engine::Nodes::Base &parent,
        const std::string &path,
        std::vector<std::string> &importStack
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

    static std::string
    resolvePath(const std::string &path, const std::string &importPath);

    std::shared_ptr<Engine::Nodes::Base> getParent() const;

    std::weak_ptr<Engine::Nodes::Base> parent;
    std::unordered_map<std::string, NodeCreator> nodeCreators;
};

} // namespace Engine
