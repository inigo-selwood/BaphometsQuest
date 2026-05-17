#pragma once

namespace Engine::Lifecycle {

/** RAII guard for engine runtime subsystems */
class Session {
  public:
    Session();
    ~Session();

    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;
};

/** Start SDL and supporting runtime subsystems */
void start();

/** Stop any runtime subsystems that have been started */
void stop();

} // namespace Engine::Lifecycle
