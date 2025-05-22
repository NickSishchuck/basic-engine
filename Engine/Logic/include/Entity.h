#ifndef ENTITY_H
#define ENTITY_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <string>
#include "Component.h"

namespace Engine {
namespace Logic {

class Entity {
private:
    std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
    std::vector<std::shared_ptr<Component>> componentsVector; // For iteration
    int id;
    bool active;
    std::string name;

    static int nextId;

public:
    // Constructors
    Entity();
    Entity(const std::string& entityName);
    Entity(int entityId, const std::string& entityName = "");
    ~Entity() = default;

    // Component Management
    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        // Check if component already exists
        std::type_index typeIndex(typeid(T));
        if (components.find(typeIndex) != components.end()) {
            // Return existing component
            return std::dynamic_pointer_cast<T>(components[typeIndex]);
        }

        // Create new component
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        component->SetOwner(this);

        // Store in both containers
        components[typeIndex] = component;
        componentsVector.push_back(component);

        // Initialize the component
        component->Initialize();

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

    template<typename T>
    bool HasComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        return components.find(std::type_index(typeid(T))) != components.end();
    }

    template<typename T>
    bool RemoveComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            // Call destroy on component
            it->second->Destroy();

            // Remove from both containers
            components.erase(it);
            componentsVector.erase(
                std::__remove_if(componentsVector.begin(), componentsVector.end(),
                    [&typeIndex](const std::shared_ptr<Component>& comp) {
                        return std::type_index(typeid(*comp)) == typeIndex;
                    }),
                componentsVector.end()
            );
            return true;
        }
        return false;
    }

    // Entity lifecycle
    void Update(float deltaTime);
    void Destroy();

    // Entity properties
    int GetID() const { return id; }
    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }

    bool IsActive() const { return active; }
    void SetActive(bool isActive) { active = isActive; }

    // Component iteration
    const std::vector<std::shared_ptr<Component>>& GetAllComponents() const {
        return componentsVector;
    }

    size_t GetComponentCount() const { return components.size(); }

    // Debug/inspection
    std::string GetDebugInfo() const;
    void PrintComponentList() const;
};

} // namespace Logic
} // namespace Engine

#endif
