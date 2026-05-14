#pragma once

#include "../node/node.hpp"

namespace Engine {

/** Countdown timer node that emits finished() when its duration elapses. */
class Timer : public Node {
  public:
    explicit Timer(float duration = 0.0F, bool repeating = false);

    /** Register the Timer XML node type. */
    static void registerType();

    /** Consume and clear a completion edge. */
    bool consumeFinished();

    /** Return elapsed seconds since the timer started. */
    float getElapsed() const;

    /** Return remaining seconds before completion. */
    float getRemaining() const;

    /** Return whether the timer completed since the last reset/start. */
    bool isFinished() const;

    /** Return whether the timer is currently running. */
    bool isRunning() const;

    /** Reset elapsed time without starting or stopping the timer. */
    void reset();

    /** Start or restart the timer. */
    void start();

    /** Stop the timer without resetting elapsed time. */
    void stop();

  private:
    void onEnterTree() override;
    void processSelf(float deltaTime);

    float duration = 0.0F;
    float elapsed = 0.0F;
    bool autoStart = true;
    bool finished = false;
    bool repeating = false;
    bool running = false;
};

} // namespace Engine
