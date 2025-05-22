#include "../include/Entity.h"
#include <iostream>
#include <algorithm>

namespace Engine {
namespace Logic {

// Static member initialization
int Entity::nextId = 1;

// Constructors
Entity::Entity()
    : id(nextId++), active(true), name("Entity_" + std::to_string(id)) {
}

Entity::Entity(const std::string& entityName)
    : id(nextId++), active(true), name(entityName) {
}

Entity::Entity(int entityId, const std::string& entityName)
    : id(entityId), active(true), name(entityName.empty() ? "Entity_" + std::to_string(id) : entityName) {
    // Update nextId if this id is higher
    if (entityId >= nextId) {
        nextId = entityId + 1;
    }
}

void Entity::Update(float deltaTime) {
    if (!active) return;

    // Update all enabled components
    for (auto& component : componentsVector) {
        if (component && component->IsEnabled()) {
            component->Update(deltaTime);
        }
    }
}

void Entity::Destroy() {
    // Call destroy on all components
    for (auto& component : componentsVector) {
        if (component) {
            component->Destroy();
        }
    }

    // Clear all component containers
    components.clear();
    componentsVector.clear();

    // Mark as inactive
    active = false;
}

std::string Entity::GetDebugInfo() const {
    std::string info = "Entity: " + name + " (ID: " + std::to_string(id) + ")\n";
    info += "Active: " + std::string(active ? "true" : "false") + "\n";
    info += "Components (" + std::to_string(components.size()) + "):\n";

    for (const auto& component : componentsVector) {
        if (component) {
            info += "  - " + component->GetTypeName() + " (Enabled: " +
                    std::string(component->IsEnabled() ? "true" : "false") + ")\n";
        }
    }

    return info;
}

void Entity::PrintComponentList() const {
    std::cout << GetDebugInfo() << std::endl;
}

} // namespace Logic
} // namespace Engine
