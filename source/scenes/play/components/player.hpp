#pragma once

#include "../../../engine/nodes/native/image.hpp"
#include "world.hpp"

#include <SDL.h>

namespace Scenes::Play::Components {

/** Controllable player image for map traversal */
class Player : public Engine::Nodes::Image {
  public:
    Player() {
        this->declareHook(Engine::Nodes::Hook::Enter);
        this->declareHook(Engine::Nodes::Hook::Exit);
        this->declareHook(Engine::Nodes::Hook::Process);
        this->declareProperty("speed", this->speed);
        this->declareProperty("step", this->step);
    }

    void enter() override {
        this->position =
            this->getGame().state.ensure("player-position", SDL_Point{80, 72});
    }

    void exit() override {
        this->getGame().state.set("player-position", this->position);
    }

    void process(float deltaSeconds) override {
        const Uint8 *keys = SDL_GetKeyboardState(nullptr);
        const SDL_Point movement = this->getMovement(keys);
        const float interval = this->getMovementInterval();

        if(interval <= 0.0F) {
            return;
        }

        this->elapsedMovementTime += deltaSeconds;

        if(this->elapsedMovementTime > interval) {
            this->elapsedMovementTime = interval;
        }

        if(movement.x == 0 && movement.y == 0) {
            return;
        }

        if(this->elapsedMovementTime < interval) {
            return;
        }

        this->elapsedMovementTime -= interval;

        const SDL_Point target{
            this->position.x + movement.x,
            this->position.y + movement.y,
        };

        if(!this->getAncestor<World>()->canMove(this->position, target)) {
            return;
        }

        this->position = target;
    }

  private:
    /** Return a cardinal grid movement from the currently held keys */
    SDL_Point getMovement(const Uint8 *keys) const {
        if(keys[SDL_SCANCODE_UP] && !keys[SDL_SCANCODE_DOWN]) {
            return SDL_Point{0, -this->step};
        }

        if(keys[SDL_SCANCODE_DOWN] && !keys[SDL_SCANCODE_UP]) {
            return SDL_Point{0, this->step};
        }

        if(keys[SDL_SCANCODE_LEFT] && !keys[SDL_SCANCODE_RIGHT]) {
            return SDL_Point{-this->step, 0};
        }

        if(keys[SDL_SCANCODE_RIGHT] && !keys[SDL_SCANCODE_LEFT]) {
            return SDL_Point{this->step, 0};
        }

        return SDL_Point{0, 0};
    }

    /** Return seconds between grid steps at the configured speed */
    float getMovementInterval() const {
        if(this->step <= 0 || this->speed <= 0) {
            return 0.0F;
        }

        return static_cast<float>(this->step)
            / static_cast<float>(this->speed);
    }

    float elapsedMovementTime = 1.0F;
    int speed = 102;
    int step = 8;
};

} // namespace Scenes::Play::Components
