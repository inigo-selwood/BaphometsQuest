#include "node_manager.hpp"

#include "runtime/game.hpp"

#include <memory>
#include <stdexcept>

namespace Engine {

NodeManager::NodeManager(Game &game) : game(game) {}

void NodeManager::setRoot(const std::shared_ptr<Node> &root) {
    if(this->root != nullptr && this->root != root) {
        this->root->attach({});
    }

    this->root = root;

    if(this->root != nullptr) {
        try {
            this->root->attach(this->game.shared_from_this());
        } catch(const std::bad_weak_ptr &) {
            throw std::runtime_error(
                "Game must be owned by std::shared_ptr before attaching nodes"
            );
        }
    }
}

void NodeManager::enter() {
    this->walk(this->root, [](Node &node) {
        if(node.hasHook(Hook::Enter)) {
            node.enter();
        }
    });
}

void NodeManager::exit() {
    this->walk(this->root, [](Node &node) {
        if(node.hasHook(Hook::Exit)) {
            node.exit();
        }
    });
}

void NodeManager::input(const SDL_Event &event) {
    this->walk(this->root, [&event](Node &node) {
        if(node.hasHook(Hook::Input)) {
            node.input(event);
        }
    });
}

void NodeManager::process(float deltaSeconds) {
    this->walk(this->root, [deltaSeconds](Node &node) {
        if(node.hasHook(Hook::Process)) {
            node.process(deltaSeconds);
        }
    });
}

void NodeManager::render(SDL_Renderer &renderer) {
    this->walk(this->root, [&renderer](Node &node) {
        if(node.hasHook(Hook::Render)) {
            node.render(renderer);
        }
    });
}

} // namespace Engine
