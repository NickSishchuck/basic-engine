#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "CollisionComponent.h"
#include "Entity.h"
#include <vector>
#include <memory>

namespace Engine {
namespace Logic {

class CollisionSystem {
private:
    std::vector<std::shared_ptr<Entity>> entities;

public:
    CollisionSystem() = default;
    ~CollisionSystem() = default;

    // Entity management
    void RegisterEntity(std::shared_ptr<Entity> entity);
    void UnregisterEntity(std::shared_ptr<Entity> entity);
    void Clear();

    // Main collision detection and resolution
    void Update(float deltaTime);

    // Static collision detection functions
    static CollisionInfo CheckCollision(CollisionComponent* colliderA, CollisionComponent* colliderB);

    // Specific collision detection algorithms
    static CollisionInfo CheckCircleCircle(const CollisionComponent* circleA, const CollisionComponent* circleB);
    static CollisionInfo CheckCircleLine(const CollisionComponent* circle, const CollisionComponent* line);
    static CollisionInfo CheckCircleAABB(const CollisionComponent* circle, const CollisionComponent* aabb);
    static CollisionInfo CheckAABBAABB(const CollisionComponent* aabbA, const CollisionComponent* aabbB);

    // Collision response
    static void ResolveCollision(const CollisionInfo& collision, CollisionComponent* colliderA, CollisionComponent* colliderB);

    // Utility functions
    static float Distance(const glm::vec2& a, const glm::vec2& b);
    static glm::vec2 Normalize(const glm::vec2& v);
    static float Dot(const glm::vec2& a, const glm::vec2& b);
    static glm::vec2 ClosestPointOnLine(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd);

    // Debug
    size_t GetEntityCount() const { return entities.size(); }
    std::string GetDebugInfo() const;
};

} // namespace Logic
} // namespace Engine

#endif
