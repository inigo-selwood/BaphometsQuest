#pragma once

#include <SDL.h>

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
    }

    /** Store string literals and string views as std::string values */
    void set(const std::string &name, std::string_view value) {
        this->values[name] = std::string{value};
    }

    /** Return a typed state value */
    template <typename Type>
        requires isSupportedValue<std::decay_t<Type>>
    const std::decay_t<Type> &get(const std::string &name) const {
        using StoredType = std::decay_t<Type>;

        const auto value = this->values.find(name);

        if(value == this->values.end()) {
            throw std::runtime_error(
                "State key '" + name + "' does not exist"
            );
        }

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
        return this->values.contains(name);
    }

    /** Return true when no state keys exist */
    bool empty() const {
        return this->values.empty();
    }

    /** Remove a state key */
    void erase(const std::string &name) {
        this->values.erase(name);
    }

    /** Remove all state keys */
    void clear() {
        this->values.clear();
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
            stream << "values: {}\n";
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

    static SDL_Point pointFromYAML(const ::YAML::Node &node) {
        if(!node.IsSequence() || node.size() != 2) {
            throw std::runtime_error(
                "State point value must be a two-item sequence"
            );
        }

        return SDL_Point{node[0].as<int>(), node[1].as<int>()};
    }

    ::YAML::Node toYAML() const {
        ::YAML::Node root;
        ::YAML::Node valuesNode{::YAML::NodeType::Map};
        std::vector<std::string> keys;

        for(const auto &[name, value] : this->values) {
            keys.push_back(name);
        }

        std::sort(keys.begin(), keys.end());

        for(const std::string &name : keys) {
            valuesNode[name] = valueToYAML(this->values.at(name));
        }

        root["values"] = valuesNode;

        return root;
    }

    void fromYAML(const ::YAML::Node &node) {
        std::unordered_map<std::string, Value> parsedValues;

        if(!node || node.IsNull()) {
            this->values.clear();
            return;
        }

        const ::YAML::Node valuesNode = node["values"];

        if(valuesNode && valuesNode.IsNull()) {
            this->values.clear();
            return;
        }

        if(!valuesNode || !valuesNode.IsMap()) {
            throw std::runtime_error("State YAML requires a values map");
        }

        for(const auto &entry : valuesNode) {
            if(!entry.first.IsScalar()) {
                throw std::runtime_error("State YAML keys must be scalars");
            }

            parsedValues[entry.first.as<std::string>()] =
                valueFromYAML(entry.second);
        }

        this->values = std::move(parsedValues);
    }

    static ::YAML::Node valueToYAML(const Value &value) {
        ::YAML::Node node;

        std::visit(
            [&node](const auto &storedValue) {
                using StoredType = std::decay_t<decltype(storedValue)>;

                if constexpr(std::is_same_v<StoredType, bool>) {
                    node["type"] = "bool";
                    node["value"] = storedValue;
                } else if constexpr(std::is_same_v<StoredType, int>) {
                    node["type"] = "int";
                    node["value"] = storedValue;
                } else if constexpr(std::is_same_v<StoredType, std::string>) {
                    node["type"] = "string";
                    node["value"] = storedValue;
                } else if constexpr(std::is_same_v<StoredType, SDL_Point>) {
                    node["type"] = "point";
                    node["value"] = pointToYAML(storedValue);
                }
            },
            value
        );

        return node;
    }

    static Value valueFromYAML(const ::YAML::Node &node) {
        if(!node.IsMap() || !node["type"] || !node["value"]) {
            throw std::runtime_error(
                "State value must contain type and value fields"
            );
        }

        const std::string type = node["type"].as<std::string>();
        const ::YAML::Node value = node["value"];

        if(type == "bool") {
            return value.as<bool>();
        }

        if(type == "int") {
            return value.as<int>();
        }

        if(type == "string") {
            return value.as<std::string>();
        }

        if(type == "point") {
            return pointFromYAML(value);
        }

        throw std::runtime_error(
            "State value type '" + type + "' is not supported"
        );
    }

    std::unordered_map<std::string, Value> values;
};

} // namespace Engine::State
