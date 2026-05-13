#pragma once

#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

namespace Engine {

/**
 * Stores typed signal registrations and callbacks for a single node.
 *
 * This class backs Node's public signal facade. Signals are registered with a
 * fixed argument list and later connections/emits must use the same types.
 */
class SignalRegistry {
  public:
    /**
     * Connect a callback to a typed signal.
     *
     * @throws std::runtime_error if the signal is unknown or has another
     * signature.
     */
    template <typename... Arguments, typename Callback>
    void connect(
        const std::string &name,
        const std::string &ownerName,
        Callback &&callback
    ) {
        auto &signal = this->get(name, ownerName);
        this->validateArguments<Arguments...>(name, ownerName, signal);

        spdlog::debug(
            "Connecting to signal '{}' on node '{}'.",
            name,
            ownerName
        );

        std::function<void(std::decay_t<Arguments>...)> signalCallback =
            std::forward<Callback>(callback);

        signal.callbacks.push_back(
            [signalCallback =
                    std::move(signalCallback)](const auto &arguments) {
                SignalRegistry::callCallback<Arguments...>(
                    signalCallback,
                    arguments,
                    std::index_sequence_for<Arguments...>{}
                );
            }
        );
    }

    /**
     * Emit a typed signal to all connected callbacks.
     *
     * @throws std::runtime_error if the signal is unknown or has another
     * signature.
     */
    template <typename... Arguments>
    void emit(
        const std::string &name,
        const std::string &ownerName,
        Arguments &&...arguments
    ) {
        auto &signal = this->get(name, ownerName);
        this->validateArguments<Arguments...>(name, ownerName, signal);

        spdlog::debug(
            "Node '{}' emitted signal '{}' with {} argument(s).",
            ownerName,
            name,
            sizeof...(Arguments)
        );

        std::vector<std::any> signalArguments;
        signalArguments.reserve(sizeof...(Arguments));
        (signalArguments.emplace_back(std::forward<Arguments>(arguments)),
            ...);

        for(const auto &signalCallback : signal.callbacks) {
            signalCallback(signalArguments);
        }
    }

    /** Return true when a signal with this name has been registered. */
    bool has(const std::string &name) const;

    /**
     * Register a signal with a fixed argument signature.
     *
     * Re-registering the same signature is allowed.
     */
    template <typename... Arguments>
    void
    registerSignal(const std::string &name, const std::string &ownerName) {
        if(this->has(name)) {
            this->validateArguments<Arguments...>(
                name,
                ownerName,
                this->signals.at(name)
            );
            return;
        }

        spdlog::debug(
            "Registering signal '{}' on node '{}'.",
            name,
            ownerName
        );
        this->signals[name] = Signal{
            {std::type_index(typeid(std::decay_t<Arguments>))...},
            {},
        };
    }

  private:
    struct Signal {
        std::vector<std::type_index> argumentTypes;
        std::vector<std::function<void(const std::vector<std::any> &)>>
            callbacks;
    };

    template <typename... Arguments, std::size_t... Indexes>
    static void callCallback(
        const std::function<void(std::decay_t<Arguments>...)> &callback,
        const std::vector<std::any> &arguments,
        std::index_sequence<Indexes...>
    ) {
        callback(
            std::any_cast<std::decay_t<Arguments>>(arguments.at(Indexes))...
        );
    }

    template <typename... Arguments>
    void validateArguments(
        const std::string &name,
        const std::string &ownerName,
        const Signal &signal
    ) const {
        const std::vector<std::type_index> argumentTypes{
            std::type_index(typeid(std::decay_t<Arguments>))...};

        if(signal.argumentTypes == argumentTypes) {
            return;
        }

        throw std::runtime_error(
            "Signal '" + name + "' argument types do not match on node '"
            + ownerName + "'."
        );
    }

    Signal &get(const std::string &name, const std::string &ownerName);
    const Signal &
    get(const std::string &name, const std::string &ownerName) const;

    std::unordered_map<std::string, Signal> signals;
};

} // namespace Engine
