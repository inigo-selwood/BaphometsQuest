#include "node.hpp"

#include "../game.hpp"

#include <memory>
#include <stdexcept>

namespace Engine::Nodes {

Manager::Manager(Game &game) : game(game) {}

void Manager::setRoot(const std::shared_ptr<Base> &root) {
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

void Manager::enter() {
    this->walk(this->root, [](Base &node) {
        node.runSetup();
    });

    this->walk(this->root, [](Base &node) {
        if(node.hasHook(Hook::Enter)) {
            node.enter();
        }
    });
}

void Manager::exit() {
    this->walk(this->root, [](Base &node) {
        if(node.hasHook(Hook::Exit)) {
            node.exit();
        }
    });
}

void Manager::input(const SDL_Event &event) {
    this->walk(this->root, [&event](Base &node) {
        if(node.hasHook(Hook::Input)) {
            node.input(event);
        }
    });
}

void Manager::process(float deltaSeconds) {
    this->walk(this->root, [deltaSeconds](Base &node) {
        if(node.hasHook(Hook::Process)) {
            node.process(deltaSeconds);
        }
    });
}

void Manager::render(SDL_Renderer &renderer) {
    this->walk(this->root, [&renderer](Base &node) {
        if(node.hasHook(Hook::Render)) {
            node.render(renderer);
        }
    });
}

} // namespace Engine::Nodes
