#include "../include/Scene.h"
#include <iostream>
#include <algorithm>

namespace Engine {
namespace Logic {

Scene::Scene(const std::string& sceneName)
    : name(sceneName), active(true) {
}

std::shared_ptr<Entity> Scene::CreateEntity(const std::string& entityName) {
    auto entity = std::make_shared<Entity>(entityName);
    entities.push_back(entity);
    return entity;
}

std::shared_ptr<Entity> Scene::CreateEntity(int id, const std::string& entityName) {
    auto entity = std::make_shared<Entity>(id, entityName);
    entities.push_back(entity);
    return entity;
}

void Scene::RemoveEntity(int entityId) {
    auto it = std::remove_if(entities.begin(), entities.end(),
        [entityId](const std::shared_ptr<Entity>& entity) {
            return entity->GetID() == entityId;
        });

    if (it != entities.end()) {
        (*it)->Destroy(); // Clean up the entity
        entities.erase(it, entities.end());
    }
}

void Scene::RemoveEntity(std::shared_ptr<Entity> entity) {
    if (!entity) return;
    RemoveEntity(entity->GetID());
}

std::shared_ptr<Entity> Scene::FindEntity(int entityId) {
    auto it = std::find_if(entities.begin(), entities.end(),
        [entityId](const std::shared_ptr<Entity>& entity) {
            return entity->GetID() == entityId;
        });

    return (it != entities.end()) ? *it : nullptr;
}

std::shared_ptr<Entity> Scene::FindEntity(const std::string& entityName) {
    auto it = std::find_if(entities.begin(), entities.end(),
        [&entityName](const std::shared_ptr<Entity>& entity) {
            return entity->GetName() == entityName;
        });

    return (it != entities.end()) ? *it : nullptr;
}

void Scene::Update(float deltaTime) {
    if (!active) return;

    // Update all active entities
    for (auto& entity : entities) {
        if (entity && entity->IsActive()) {
            entity->Update(deltaTime);
        }
    }

    // Remove destroyed entities
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::shared_ptr<Entity>& entity) {
                return !entity || !entity->IsActive();
            }),
        entities.end()
    );
}

void Scene::Destroy() {
    for (auto& entity : entities) {
        if (entity) {
            entity->Destroy();
        }
    }
    entities.clear();
    active = false;
}

void Scene::PrintEntityList() const {
    std::cout << GetDebugInfo() << std::endl;
}

std::string Scene::GetDebugInfo() const {
    std::string info = "Scene: " + name + "\n";
    info += "Active: " + std::string(active ? "true" : "false") + "\n";
    info += "Entities (" + std::to_string(entities.size()) + "):\n";

    for (const auto& entity : entities) {
        if (entity) {
            info += "  - " + entity->GetName() + " (ID: " +
                   std::to_string(entity->GetID()) + ", Components: " +
                   std::to_string(entity->GetComponentCount()) + ")\n";
        }
    }

    return info;
}

} // namespace Logic
} // namespace Engine
