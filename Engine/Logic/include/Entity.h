#ifndef ENTITY_H
#define ENTITY_H

#include <unordered_map>
#include <memory>
#include <typeindex>
#include "Component.h"

namespace Engine {
namespace Logic {

class Entity {
private:
    std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
    int id;
    bool active;

public:
    Entity(int id);
    ~Entity() = default;

    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        components[std::type_index(typeid(T))] = component;
        return component;
    }

    template<typename T>
    std::shared_ptr<T> GetComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        auto it = components.find(std::type_index(typeid(T)));
        if (it != components.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    void Update(float deltaTime);
    int GetID() const { return id; }
    bool IsActive() const { return active; }
    void SetActive(bool isActive) { active = isActive; }
};

} // namespace Logic
} // namespace Engine

#endif
