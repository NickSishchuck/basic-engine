#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"
#include <vector>
#include <memory>
#include <string>

namespace Engine {
namespace Logic {

class Scene {
private:
    std::vector<std::shared_ptr<Entity>> entities;
    std::string name;
    bool active;

public:
    Scene(const std::string& sceneName = "Untitled Scene");
    ~Scene() = default;

    // Entity management
    std::shared_ptr<Entity> CreateEntity(const std::string& name = "");
    std::shared_ptr<Entity> CreateEntity(int id, const std::string& name = "");
    void RemoveEntity(int entityId);
    void RemoveEntity(std::shared_ptr<Entity> entity);
    std::shared_ptr<Entity> FindEntity(int entityId);
    std::shared_ptr<Entity> FindEntity(const std::string& name);

    // Scene lifecycle
    void Update(float deltaTime);
    void Destroy();

    // Scene properties
    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }
    bool IsActive() const { return active; }
    void SetActive(bool isActive) { active = isActive; }

    // Entity access
    const std::vector<std::shared_ptr<Entity>>& GetEntities() const { return entities; }
    size_t GetEntityCount() const { return entities.size(); }

    // Debug
    void PrintEntityList() const;
    std::string GetDebugInfo() const;
};

} // namespace Logic
} // namespace Engine

#endif
