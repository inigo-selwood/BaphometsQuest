#pragma once

#include "../assets/assetRegistry.hpp"

#include "../node/node.hpp"

#include <string>

namespace Engine {

/**
 * Load XML scene children into a scene root node.
 *
 * The scene file must have a <Scene> root. Child elements map to registered
 * node types, and XML attributes map to node properties.
 */
void loadSceneFromXML(
    Node &scene,
    AssetRegistry &assets,
    const std::string &path
);

} // namespace Engine
