#pragma once

#include <SDL.h>

namespace Engine {

/** Adaptive frame limiter for the runtime loop */
class Timer {
  public:
    /** Lowest active frame rate the timer can adjust down to */
    static constexpr int MIN_FRAME_RATE = 12;

    /** Load the configured target frame rate, clamping below the minimum */
    void setTargetFrameRate(int frameRate);

    /** Record frame work duration and adjust the cap when there is room */
    void recordFrameDuration(Uint32 frameDuration);

    /** Return the delay needed to reach the active frame budget */
    Uint32 getDelayDuration(Uint32 frameDuration) const;

  private:
    /** Return the current active frame budget in milliseconds */
    float getTargetFrameDuration() const;

    /** Return true when target FPS leaves enough room for adaptive changes */
    bool canAdjustFrameRate() const;

    /** Configured frame-rate cap loaded from settings */
    int targetFrameRate = 0;

    /** Current frame-rate cap after adaptive adjustment */
    int activeFrameRate = 0;

    /** Smoothed frame work duration in milliseconds */
    float averageFrameDuration = 0.0F;
};

} // namespace Engine
