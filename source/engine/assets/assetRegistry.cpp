#include "assetRegistry.hpp"

namespace Engine {

void AssetRegistry::clear() {
    this->unloadAll();
}

} // namespace Engine
