#include "timer.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

namespace Engine {

void Timer::setTargetFrameRate(int frameRate) {
    if(frameRate < MIN_FRAME_RATE) {
        spdlog::warn(
            "Target frame rate {} FPS is below minimum; using {} FPS",
            frameRate,
            MIN_FRAME_RATE
        );
        frameRate = MIN_FRAME_RATE;
    }

    this->targetFrameRate = frameRate;
    this->activeFrameRate = frameRate;
    this->averageFrameDuration = this->getTargetFrameDuration();
}

void Timer::recordFrameDuration(Uint32 frameDuration) {
    constexpr float EMA_ALPHA = 0.10F;
    constexpr float SLOW_THRESHOLD = 1.10F;
    constexpr float STABLE_THRESHOLD = 0.85F;

    if(!this->canAdjustFrameRate()) {
        return;
    }

    const float measuredFrameDuration = static_cast<float>(frameDuration);
    this->averageFrameDuration =
        (this->averageFrameDuration * (1.0F - EMA_ALPHA))
        + (measuredFrameDuration * EMA_ALPHA);

    const int previousFrameRate = this->activeFrameRate;
    const float targetFrameDuration = this->getTargetFrameDuration();

    if(this->averageFrameDuration
        > targetFrameDuration * SLOW_THRESHOLD) {
        this->activeFrameRate =
            std::max(MIN_FRAME_RATE, this->activeFrameRate - 1);
    } else if(
        this->averageFrameDuration < targetFrameDuration * STABLE_THRESHOLD
    ) {
        this->activeFrameRate =
            std::min(this->targetFrameRate, this->activeFrameRate + 1);
    }

    if(this->activeFrameRate != previousFrameRate) {
        spdlog::debug(
            "Adjusted frame rate to {} FPS",
            this->activeFrameRate
        );
    }
}

Uint32 Timer::getDelayDuration(Uint32 frameDuration) const {
    const float targetFrameDuration = this->getTargetFrameDuration();

    if(static_cast<float>(frameDuration) >= targetFrameDuration) {
        return 0;
    }

    return static_cast<Uint32>(
        targetFrameDuration - static_cast<float>(frameDuration)
    );
}

float Timer::getTargetFrameDuration() const {
    return 1000.0F / static_cast<float>(this->activeFrameRate);
}

bool Timer::canAdjustFrameRate() const {
    constexpr int MIN_ADJUSTMENT_RANGE = 2;

    return this->targetFrameRate - MIN_FRAME_RATE >= MIN_ADJUSTMENT_RANGE;
}

} // namespace Engine
