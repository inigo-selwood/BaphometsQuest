#pragma once

#include "node.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine {

/**
 * Global registry that maps XML node type names to factories.
 *
 * Node keeps the public registration/create facade; this class owns the
 * backing storage.
 */
class NodeRegistry {
  public:
    /** Function that creates a detached node instance. */
    using NodeFactory = std::function<std::unique_ptr<Node>()>;

    /** Return the process-wide node registry. */
    static NodeRegistry &getInstance();

    /** Create a registered node type by name. */
    std::unique_ptr<Node> create(const std::string &name) const;

    /** Register or replace a node factory by XML type name. */
    void registerType(const std::string &name, NodeFactory factory);

  private:
    std::unordered_map<std::string, NodeFactory> factories;
};

} // namespace Engine
