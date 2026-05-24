#include "node.hpp"

#include "../../nodes/native/camera.hpp"
#include "../game.hpp"
#include "../render/canvas.hpp"

#include <memory>
#include <optional>
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
    this->walkAll(this->root, [](Base &node) { node.runSetup(); });

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
    std::vector<std::shared_ptr<Base>> inputNodes;
    this->collectInputNodes(this->root, inputNodes);

    for(const std::shared_ptr<Base> &node : inputNodes) {
        node->input(event);
    }
}

void Manager::process(float deltaSeconds) {
    this->walk(this->root, [deltaSeconds](Base &node) {
        if(node.hasHook(Hook::Process)) {
            node.process(deltaSeconds);
        }
    });
}

void Manager::render(SDL_Renderer &renderer) {
    Engine::Render::Context context;
    context.screenBounds = this->game.getScreenSize();

    // Missing canvas layers intentionally use the screen-space context default
    // Missing cameras intentionally leave world layers at origin viewport
    std::optional<SDL_Point> activeCameraFocus;
    this->findActiveCamera(this->root, context, activeCameraFocus);

    if(activeCameraFocus.has_value()) {
        context.viewportOrigin = Engine::Render::Context::centreViewportOn(
            *activeCameraFocus,
            context.screenBounds
        );
    }

    this->renderNode(this->root, context, renderer);
}

void Manager::collectInputNodes(
    const std::shared_ptr<Base> &node,
    std::vector<std::shared_ptr<Base>> &nodes
) const {
    if(node == nullptr) {
        return;
    }

    if(!node->active) {
        return;
    }

    if(node->hasHook(Hook::Input)) {
        nodes.push_back(node);
    }

    for(std::size_t index = 0; index < node->children.size(); index++) {
        this->collectInputNodes(node->children[index], nodes);
    }
}

void Manager::findActiveCamera(
    const std::shared_ptr<Base> &node,
    Engine::Render::Context context,
    std::optional<SDL_Point> &cameraFocus
) const {
    if(node == nullptr) {
        return;
    }

    if(!node->active) {
        return;
    }

    node->applyRenderContext(context);

    const std::shared_ptr<Camera> camera =
        std::dynamic_pointer_cast<Camera>(node);

    if(camera != nullptr && camera->isActive()
        && context.mode == Engine::Render::CanvasMode::World) {
        if(cameraFocus.has_value()) {
            throw std::runtime_error("Multiple active world cameras found");
        }

        cameraFocus = context.origin;
    }

    for(std::size_t index = 0; index < node->children.size(); index++) {
        this->findActiveCamera(node->children[index], context, cameraFocus);
    }
}

void Manager::renderNode(
    const std::shared_ptr<Base> &node,
    Engine::Render::Context context,
    SDL_Renderer &renderer
) const {
    if(node == nullptr) {
        return;
    }

    if(!node->active) {
        return;
    }

    node->applyRenderContext(context);

    if(node->hasHook(Hook::Render)) {
        Engine::Render::Canvas canvas{renderer, context};
        node->render(canvas);
    }

    for(std::size_t index = 0; index < node->children.size(); index++) {
        this->renderNode(node->children[index], context, renderer);
    }
}

} // namespace Engine::Nodes
