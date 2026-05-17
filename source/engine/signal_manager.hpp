#pragma once

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

namespace Engine {

class Node;

} // namespace Engine

namespace Engine::Signal {

/** Node-scoped signal bus with runtime signature checks */
class Manager {
  public:
    /**
     * Connect a callback to a signal registered on a node
     *
     * @throws std::runtime_error when the signal is unknown or has a different
     * signature
     */
    template <typename... Arguments, typename Callback>
    void connect(
        const std::shared_ptr<Engine::Node> &owner,
        const std::string &name,
        Callback &&callback
    ) {
        Signal &signal = this->get(owner, name);
        this->validateArguments<Arguments...>(name, signal);

        std::function<void(std::decay_t<Arguments>...)> signalCallback =
            std::forward<Callback>(callback);

        signal.callbacks.push_back(
            [signalCallback =
                    std::move(signalCallback)](const auto &arguments) {
                Manager::invokeCallback<Arguments...>(
                    signalCallback,
                    arguments,
                    std::index_sequence_for<Arguments...>{}
                );
            }
        );
    }

    /**
     * Emit a signal registered on a node to all connected callbacks
     *
     * @throws std::runtime_error when the signal is unknown or has a different
     * signature
     */
    template <typename... Arguments>
    void emit(
        const std::shared_ptr<Engine::Node> &owner,
        const std::string &name,
        Arguments &&...arguments
    ) {
        Signal &signal = this->get(owner, name);
        this->validateArguments<Arguments...>(name, signal);

        std::vector<std::any> signalArguments;
        signalArguments.reserve(sizeof...(Arguments));
        (signalArguments.emplace_back(std::forward<Arguments>(arguments)),
            ...);

        for(const auto &callback : signal.callbacks) {
            callback(signalArguments);
        }
    }

    /** Return true when a signal has been registered on a node */
    bool
    has(const std::shared_ptr<Engine::Node> &owner,
        const std::string &name) const {
        this->validateOwner(owner);

        const auto ownerSignals = this->signals.find(owner);

        if(ownerSignals == this->signals.end()) {
            return false;
        }

        return ownerSignals->second.contains(name);
    }

    /**
     * Register a node-scoped signal with a fixed argument signature
     *
     * Re-registering with the same signature is allowed
     *
     * @throws std::runtime_error when the signal already exists with a
     * different signature
     */
    template <typename... Arguments>
    void declare(
        const std::shared_ptr<Engine::Node> &owner,
        const std::string &name
    ) {
        this->validateOwner(owner);

        if(this->has(owner, name)) {
            this->validateArguments<Arguments...>(
                name,
                this->signals.at(owner).at(name)
            );
            return;
        }

        this->signals[owner][name] = Signal{
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

    Signal &
    get(const std::shared_ptr<Engine::Node> &owner, const std::string &name) {
        if(!this->has(owner, name)) {
            throw std::runtime_error("Unknown signal '" + name + "'");
        }

        return this->signals.at(owner).at(name);
    }

    const Signal &
    get(const std::shared_ptr<Engine::Node> &owner,
        const std::string &name) const {
        if(!this->has(owner, name)) {
            throw std::runtime_error("Unknown signal '" + name + "'");
        }

        return this->signals.at(owner).at(name);
    }

    template <typename... Arguments, std::size_t... Indexes>
    static void invokeCallback(
        const std::function<void(std::decay_t<Arguments>...)> &callback,
        const std::vector<std::any> &arguments,
        std::index_sequence<Indexes...>
    ) {
        callback(
            std::any_cast<std::decay_t<Arguments>>(arguments.at(Indexes))...
        );
    }

    template <typename... Arguments>
    void
    validateArguments(const std::string &name, const Signal &signal) const {
        const std::vector<std::type_index> argumentTypes{
            std::type_index(typeid(std::decay_t<Arguments>))...};

        if(signal.argumentTypes == argumentTypes) {
            return;
        }

        throw std::runtime_error(
            "Signal '" + name + "' argument types do not match"
        );
    }

    static void validateOwner(const std::shared_ptr<Engine::Node> &owner) {
        if(owner == nullptr) {
            throw std::runtime_error("Signal owner must not be null");
        }
    }

    std::map<
        std::weak_ptr<Engine::Node>,
        std::map<std::string, Signal>,
        std::owner_less<>>
        signals;
};

} // namespace Engine::Signal
