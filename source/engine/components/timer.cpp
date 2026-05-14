#include "timer.hpp"

#include <algorithm>

namespace Engine {

Timer::Timer(float duration, bool repeating)
    : duration(std::max(0.0F, duration)), repeating(repeating) {
    this->registerProperty<bool>(
        "auto-start",
        "Timer",
        [this] { return this->autoStart; },
        [this](const bool &value) { this->autoStart = value; }
    );
    this->registerProperty<float>(
        "duration",
        "Timer",
        [this] { return this->duration; },
        [this](const float &value) { this->duration = std::max(0.0F, value); }
    );
    this->registerProperty<bool>(
        "repeating",
        "Timer",
        [this] { return this->repeating; },
        [this](const bool &value) { this->repeating = value; }
    );
    this->registerProperty<bool>(
        "running",
        "Timer",
        [this] { return this->running; },
        [this](const bool &value) { this->running = value; }
    );
    this->setProcessFunction([this](float deltaTime) {
        this->processSelf(deltaTime);
    });
}

void Timer::registerType() {
    Engine::Node::registerType<Timer>("Timer");
}

bool Timer::consumeFinished() {
    const bool wasFinished = this->finished;
    this->finished = false;
    return wasFinished;
}

float Timer::getElapsed() const {
    return this->elapsed;
}

float Timer::getRemaining() const {
    return std::max(0.0F, this->duration - this->elapsed);
}

bool Timer::isFinished() const {
    return this->finished;
}

bool Timer::isRunning() const {
    return this->running;
}

void Timer::onEnterTree() {
    this->registerSignal<>("finished");

    if(this->autoStart) {
        this->start();
    }
}

void Timer::processSelf(float deltaTime) {
    if(!this->running) {
        return;
    }

    this->elapsed += std::max(0.0F, deltaTime);

    if(this->elapsed < this->duration) {
        return;
    }

    this->finished = true;
    this->emitSignal("finished");

    if(this->repeating && this->duration > 0.0F) {
        while(this->elapsed >= this->duration) {
            this->elapsed -= this->duration;
        }
        return;
    }

    this->elapsed = this->duration;
    this->running = false;
}

void Timer::reset() {
    this->elapsed = 0.0F;
    this->finished = false;
}

void Timer::start() {
    this->reset();
    this->running = true;
}

void Timer::stop() {
    this->running = false;
}

} // namespace Engine
