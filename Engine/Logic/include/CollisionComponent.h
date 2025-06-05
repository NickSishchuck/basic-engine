#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include "Component.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Engine {
namespace Logic {

enum class CollisionShape {
    CIRCLE,
    AABB,           // Axis-Aligned Bounding Box
    LINE_SEGMENT,   // For walls/boundaries
    POINT
};

// Collision data structures
struct CircleCollider {
    float radius;
    glm::vec2 offset; // Offset from entity position

    CircleCollider(float r = 1.0f, glm::vec2 off = glm::vec2(0.0f))
        : radius(r), offset(off) {}
};

struct AABBCollider {
    glm::vec2 size;   // Width and height
    glm::vec2 offset; // Offset from entity position

    AABBCollider(glm::vec2 s = glm::vec2(1.0f), glm::vec2 off = glm::vec2(0.0f))
        : size(s), offset(off) {}
};

struct LineCollider {
    glm::vec2 start;
    glm::vec2 end;
    float thickness; // For collision detection

    LineCollider(glm::vec2 s = glm::vec2(0.0f), glm::vec2 e = glm::vec2(1.0f, 0.0f), float t = 0.1f)
        : start(s), end(e), thickness(t) {}
};

// Collision result data
struct CollisionInfo {
    bool hasCollision = false;
    glm::vec2 contactPoint = glm::vec2(0.0f);
    glm::vec2 normal = glm::vec2(0.0f);      // Direction to separate objects
    float penetration = 0.0f;                // How much objects overlap
    Entity* otherEntity = nullptr;

    CollisionInfo() = default;
    CollisionInfo(bool collision, glm::vec2 point, glm::vec2 norm, float pen, Entity* other = nullptr)
        : hasCollision(collision), contactPoint(point), normal(norm), penetration(pen), otherEntity(other) {}
};

class CollisionComponent : public Component {
private:
    CollisionShape shape;
    bool isTrigger;           // If true, detects collision but doesn't resolve
    bool isStatic;            // If true, object doesn't move from collisions
    std::string collisionLayer; // For filtering collisions

    // Shape data (only one will be used based on shape type)
    CircleCollider circleData;
    AABBCollider aabbData;
    LineCollider lineData;

    // Collision results from last frame
    std::vector<CollisionInfo> collisions;

public:
    CollisionComponent(CollisionShape shapeType = CollisionShape::CIRCLE,
                      bool trigger = false,
                      bool staticObj = false,
                      const std::string& layer = "default");

    // Component interface
    void Update(float deltaTime) override;
    std::string GetTypeName() const override { return "CollisionComponent"; }
    std::string GetDebugInfo() const override;

    // Shape configuration
    void SetShape(CollisionShape shapeType) { shape = shapeType; }
    CollisionShape GetShape() const { return shape; }

    // Circle collider
    void SetCircle(float radius, glm::vec2 offset = glm::vec2(0.0f));
    const CircleCollider& GetCircle() const { return circleData; }

    // AABB collider
    void SetAABB(glm::vec2 size, glm::vec2 offset = glm::vec2(0.0f));
    const AABBCollider& GetAABB() const { return aabbData; }

    // Line collider
    void SetLine(glm::vec2 start, glm::vec2 end, float thickness = 0.1f);
    const LineCollider& GetLine() const { return lineData; }

    // Properties
    void SetTrigger(bool trigger) { isTrigger = trigger; }
    bool IsTrigger() const { return isTrigger; }

    void SetStatic(bool staticObj) { isStatic = staticObj; }
    bool IsStatic() const { return isStatic; }

    void SetLayer(const std::string& layer) { collisionLayer = layer; }
    const std::string& GetLayer() const { return collisionLayer; }

    // Collision results
    const std::vector<CollisionInfo>& GetCollisions() const { return collisions; }
    bool HasCollisions() const { return !collisions.empty(); }
    void ClearCollisions() { collisions.clear(); }
    void AddCollision(const CollisionInfo& collision) { collisions.push_back(collision); }

    // World position helpers (combines entity position with collider offset)
    glm::vec2 GetWorldPosition() const;
    glm::vec2 GetWorldCenter() const; // For circles and AABBs
};

} // namespace Logic
} // namespace Engine

#endif
