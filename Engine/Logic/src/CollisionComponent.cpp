#include "../include/CollisionSystem.h"
#include "../include/Entity.h"
#include "../include/SimplePhysicsComponent.h"
#include "../include/TransformComponent.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace Engine {
namespace Logic {

CollisionComponent::CollisionComponent(CollisionShape shapeType, bool trigger, bool staticObj, const std::string& layer)
    : shape(shapeType), isTrigger(trigger), isStatic(staticObj), collisionLayer(layer) {

    // Initialize default shapes
    circleData = CircleCollider(1.0f, glm::vec2(0.0f));
    aabbData = AABBCollider(glm::vec2(1.0f), glm::vec2(0.0f));
    lineData = LineCollider(glm::vec2(0.0f), glm::vec2(1.0f, 0.0f), 0.1f);
}

void CollisionComponent::Update(float deltaTime) {
    // Clear collisions from previous frame
    // Note: The collision system will populate this during its update
    // We don't clear here because multiple collision checks might happen per frame
}

std::string CollisionComponent::GetDebugInfo() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    // Shape info
    std::string shapeStr;
    switch (shape) {
        case CollisionShape::CIRCLE: shapeStr = "Circle"; break;
        case CollisionShape::AABB: shapeStr = "AABB"; break;
        case CollisionShape::LINE_SEGMENT: shapeStr = "Line"; break;
        case CollisionShape::POINT: shapeStr = "Point"; break;
    }

    oss << "Shape: " << shapeStr << "\n";
    oss << "Layer: " << collisionLayer << "\n";
    oss << "Trigger: " << (isTrigger ? "true" : "false") << "\n";
    oss << "Static: " << (isStatic ? "true" : "false") << "\n";

    // Shape-specific data
    switch (shape) {
        case CollisionShape::CIRCLE:
            oss << "Radius: " << circleData.radius << "\n";
            oss << "Offset: (" << circleData.offset.x << ", " << circleData.offset.y << ")";
            break;

        case CollisionShape::AABB:
            oss << "Size: (" << aabbData.size.x << ", " << aabbData.size.y << ")\n";
            oss << "Offset: (" << aabbData.offset.x << ", " << aabbData.offset.y << ")";
            break;

        case CollisionShape::LINE_SEGMENT:
            oss << "Start: (" << lineData.start.x << ", " << lineData.start.y << ")\n";
            oss << "End: (" << lineData.end.x << ", " << lineData.end.y << ")\n";
            oss << "Thickness: " << lineData.thickness;
            break;

        default:
            break;
    }

    oss << "\nCollisions: " << collisions.size();

    return oss.str();
}

void CollisionComponent::SetCircle(float radius, glm::vec2 offset) {
    shape = CollisionShape::CIRCLE;
    circleData.radius = radius;
    circleData.offset = offset;
}

void CollisionComponent::SetAABB(glm::vec2 size, glm::vec2 offset) {
    shape = CollisionShape::AABB;
    aabbData.size = size;
    aabbData.offset = offset;
}

void CollisionComponent::SetLine(glm::vec2 start, glm::vec2 end, float thickness) {
    shape = CollisionShape::LINE_SEGMENT;
    lineData.start = start;
    lineData.end = end;
    lineData.thickness = thickness;
}

glm::vec2 CollisionComponent::GetWorldPosition() const {
    if (!owner) return glm::vec2(0.0f);

    auto transform = owner->GetComponent<TransformComponent>();
    if (!transform) return glm::vec2(0.0f);

    // For 2D, we just use X and Y components of the 3D position
    glm::vec3 pos3D = transform->GetPosition();
    return glm::vec2(pos3D.x, pos3D.y);
}

glm::vec2 CollisionComponent::GetWorldCenter() const {
    glm::vec2 worldPos = GetWorldPosition();

    switch (shape) {
        case CollisionShape::CIRCLE:
            return worldPos + circleData.offset;

        case CollisionShape::AABB:
            return worldPos + aabbData.offset;

        case CollisionShape::LINE_SEGMENT:
            return worldPos + (lineData.start + lineData.end) * 0.5f;

        default:
            return worldPos;
    }
}

} // namespace Logic
} // namespace Engine
