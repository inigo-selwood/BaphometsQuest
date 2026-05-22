#pragma once

#include <SDL.h>

#include "../utils/parse.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace Engine::State {

/** In-memory state store that survives scene switches */
class Store {
  private:
    using Value = std::variant<bool, int, std::string, SDL_Point>;

    template <typename Type>
    static constexpr bool isSupportedValue =
        std::is_same_v<Type, bool> || std::is_same_v<Type, int>
        || std::is_same_v<Type, std::string>
        || std::is_same_v<Type, SDL_Point>;

  public:
    /** Store a boolean gameplay flag */
    void setFlag(const std::string &name, bool value = true) {
        this->set(name, value);
    }

    /** Return a gameplay flag, defaulting to false when absent */
    bool getFlag(const std::string &name) const {
        if(!this->has(name)) {
            return false;
        }

        return this->get<bool>(name);
    }

    /** Store a typed state value */
    template <typename Type>
        requires isSupportedValue<std::decay_t<Type>>
    void set(const std::string &name, Type value) {
        using StoredType = std::decay_t<Type>;

        this->values[name] = StoredType{value};
        this->rawValues.erase(name);
    }

    /** Store string literals and string views as std::string values */
    void set(const std::string &name, std::string_view value) {
        this->values[name] = std::string{value};
        this->rawValues.erase(name);
    }

    /** Return a typed state value */
    template <typename Type>
        requires isSupportedValue<std::decay_t<Type>>
    const std::decay_t<Type> &get(const std::string &name) const {
        using StoredType = std::decay_t<Type>;

        const auto value = this->values.find(name);

        if(value != this->values.end()) {
            const StoredType *storedValue = std::get_if<StoredType>(
                &value->second
            );

            if(storedValue == nullptr) {
                throw std::runtime_error(
                    "State key '" + name + "' type does not match"
                );
            }

            return *storedValue;
        }

        const auto rawValue = this->rawValues.find(name);

        if(rawValue == this->rawValues.end()) {
            throw std::runtime_error(
                "State key '" + name + "' does not exist"
            );
        }

        const Value parsedValue = valueFromYAML<StoredType>(
            rawValue->second,
            name
        );
        const auto [storedValue, inserted] =
            this->values.emplace(name, parsedValue);
        this->rawValues.erase(rawValue);

        return std::get<StoredType>(storedValue->second);
    }

    /** Ensure a typed state value exists, then return it */
    template <typename Type>
        requires isSupportedValue<std::decay_t<Type>>
    const std::decay_t<Type> &
    ensure(const std::string &name, const Type &defaultValue) {
        if(!this->has(name)) {
            this->set(name, defaultValue);
        }

        return this->get<std::decay_t<Type>>(name);
    }

    /** Return true when a state key exists */
    bool has(const std::string &name) const {
        return this->values.contains(name) || this->rawValues.contains(name);
    }

    /** Return true when no state keys exist */
    bool empty() const {
        return this->values.empty() && this->rawValues.empty();
    }

    /** Remove a state key */
    void erase(const std::string &name) {
        this->values.erase(name);
        this->rawValues.erase(name);
    }

    /** Remove all state keys */
    void clear() {
        this->values.clear();
        this->rawValues.clear();
    }

    /** Save the current state */
    void save(const std::filesystem::path &path) const {
        std::ofstream stream{path};

        if(!stream) {
            throw std::runtime_error(
                "Failed to open state file '" + path.string() + "'"
            );
        }

        if(this->empty()) {
            stream << "{}\n";
            return;
        }

        stream << this->toYAML();
    }

    /** Load state from storage */
    void load(const std::filesystem::path &path) {
        this->fromYAML(::YAML::LoadFile(path.string()));
    }

  private:
    static ::YAML::Node pointToYAML(SDL_Point point) {
        ::YAML::Node node;
        node.SetStyle(::YAML::EmitterStyle::Flow);
        node.push_back(point.x);
        node.push_back(point.y);

        return node;
    }

    ::YAML::Node toYAML() const {
        ::YAML::Node root;
        std::vector<std::string> keys;

        for(const auto &[name, value] : this->values) {
            keys.push_back(name);
        }

        for(const auto &[name, value] : this->rawValues) {
            keys.push_back(name);
        }

        std::sort(keys.begin(), keys.end());

        for(const std::string &name : keys) {
            const auto value = this->values.find(name);

            if(value != this->values.end()) {
                root[name] = valueToYAML(value->second);
                continue;
            }

            root[name] = this->rawValues.at(name);
        }

        return root;
    }

    void fromYAML(const ::YAML::Node &node) {
        std::unordered_map<std::string, ::YAML::Node> parsedValues;

        if(!node || node.IsNull()) {
            this->values.clear();
            this->rawValues.clear();
            return;
        }

        if(!node.IsMap()) {
            throw std::runtime_error("State YAML root must be a map");
        }

        for(const auto &entry : node) {
            if(!entry.first.IsScalar()) {
                throw std::runtime_error("State YAML keys must be scalars");
            }

            parsedValues[entry.first.as<std::string>()] = entry.second;
        }

        this->values.clear();
        this->rawValues = std::move(parsedValues);
    }

    static ::YAML::Node valueToYAML(const Value &value) {
        ::YAML::Node node;

        std::visit(
            [&node](const auto &storedValue) {
                using StoredType = std::decay_t<decltype(storedValue)>;

                if constexpr(std::is_same_v<StoredType, SDL_Point>) {
                    node = pointToYAML(storedValue);
                } else {
                    node = storedValue;
                }
            },
            value
        );

        return node;
    }

    template <typename Type>
        requires isSupportedValue<Type>
    static Value valueFromYAML(
        const ::YAML::Node &node,
        const std::string &name
    ) {
        try {
            if constexpr(std::is_same_v<Type, SDL_Point>) {
                return Engine::Parse::point(node);
            } else if constexpr(std::is_same_v<Type, bool>) {
                return Engine::Parse::boolean(node);
            } else if constexpr(std::is_same_v<Type, int>) {
                return Engine::Parse::integer(node);
            } else if constexpr(std::is_same_v<Type, std::string>) {
                return Engine::Parse::string(node);
            } else {
                static_assert(isSupportedValue<Type>);
            }
        } catch(const std::exception &exception) {
            throw std::runtime_error(
                "State key '" + name + "' type does not match: "
                + exception.what()
            );
        }
    }

    mutable std::unordered_map<std::string, Value> values;
    mutable std::unordered_map<std::string, ::YAML::Node> rawValues;
};

} // namespace Engine::State
