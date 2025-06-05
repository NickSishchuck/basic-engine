#include "../include/CollisionSystem.h"
#include "../include/SimplePhysicsComponent.h"
#include "../include/TransformComponent.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace Engine {
namespace Logic {

void CollisionSystem::RegisterEntity(std::shared_ptr<Entity> entity) {
    if (!entity || !entity->HasComponent<CollisionComponent>()) return;

    // Check if already registered
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it == entities.end()) {
        entities.push_back(entity);
    }
}


void CollisionSystem::UnregisterEntity(std::shared_ptr<Entity> entity) {
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end()) {
        entities.erase(it);
    }
}

void CollisionSystem::Clear() {
    entities.clear();
}

void CollisionSystem::Update(float deltaTime) {
    (void)deltaTime; // Suppress unused parameter warning

    // Clear all collision results from previous frame
    for (auto& entity : entities) {
        if (!entity || !entity->IsActive()) continue;

        auto collider = entity->GetComponent<CollisionComponent>();
        if (collider) {
            collider->ClearCollisions();
        }
    }

    // Check collisions between all pairs
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            auto entityA = entities[i];
            auto entityB = entities[j];

            if (!entityA || !entityB || !entityA->IsActive() || !entityB->IsActive()) continue;

            auto colliderA = entityA->GetComponent<CollisionComponent>();
            auto colliderB = entityB->GetComponent<CollisionComponent>();

            if (!colliderA || !colliderB) continue;

            // Check collision
            CollisionInfo collision = CheckCollision(colliderA.get(), colliderB.get());

            if (collision.hasCollision) {
                // Add collision info to both entities
                collision.otherEntity = entityB.get();
                colliderA->AddCollision(collision);

                // Create reverse collision info for entity B
                CollisionInfo reverseCollision = collision;
                reverseCollision.normal = -collision.normal;
                reverseCollision.otherEntity = entityA.get();
                colliderB->AddCollision(reverseCollision);

                // Resolve collision if neither is a trigger
                if (!colliderA->IsTrigger() && !colliderB->IsTrigger()) {
                    ResolveCollision(collision, colliderA.get(), colliderB.get());
                }
            }
        }
    }
}

CollisionInfo CollisionSystem::CheckCollision(CollisionComponent* colliderA, CollisionComponent* colliderB) {
    if (!colliderA || !colliderB) return CollisionInfo();

    CollisionShape shapeA = colliderA->GetShape();
    CollisionShape shapeB = colliderB->GetShape();

    // Circle-Circle collision
    if (shapeA == CollisionShape::CIRCLE && shapeB == CollisionShape::CIRCLE) {
        return CheckCircleCircle(colliderA, colliderB);
    }

    // Circle-Line collision
    if (shapeA == CollisionShape::CIRCLE && shapeB == CollisionShape::LINE_SEGMENT) {
        return CheckCircleLine(colliderA, colliderB);
    }
    if (shapeA == CollisionShape::LINE_SEGMENT && shapeB == CollisionShape::CIRCLE) {
        CollisionInfo result = CheckCircleLine(colliderB, colliderA);
        result.normal = -result.normal; // Flip normal
        return result;
    }

    // Circle-AABB collision
    if (shapeA == CollisionShape::CIRCLE && shapeB == CollisionShape::AABB) {
        return CheckCircleAABB(colliderA, colliderB);
    }
    if (shapeA == CollisionShape::AABB && shapeB == CollisionShape::CIRCLE) {
        CollisionInfo result = CheckCircleAABB(colliderB, colliderA);
        result.normal = -result.normal; // Flip normal
        return result;
    }

    // AABB-AABB collision
    if (shapeA == CollisionShape::AABB && shapeB == CollisionShape::AABB) {
        return CheckAABBAABB(colliderA, colliderB);
    }

    // No collision detection implemented for this combination
    return CollisionInfo();
}

CollisionInfo CollisionSystem::CheckCircleCircle(const CollisionComponent* circleA, const CollisionComponent* circleB) {
    glm::vec2 centerA = circleA->GetWorldCenter();
    glm::vec2 centerB = circleB->GetWorldCenter();

    float radiusA = circleA->GetCircle().radius;
    float radiusB = circleB->GetCircle().radius;

    glm::vec2 direction = centerB - centerA;
    float distance = Distance(centerA, centerB);
    float combinedRadius = radiusA + radiusB;

    if (distance < combinedRadius) {
        // Collision detected
        float penetration = combinedRadius - distance;
        glm::vec2 normal = (distance > 0.001f) ? Normalize(direction) : glm::vec2(1.0f, 0.0f);
        glm::vec2 contactPoint = centerA + normal * radiusA;

        return CollisionInfo(true, contactPoint, normal, penetration);
    }

    return CollisionInfo();
}

CollisionInfo CollisionSystem::CheckCircleLine(const CollisionComponent* circle, const CollisionComponent* line) {
    glm::vec2 circleCenter = circle->GetWorldCenter();
    float radius = circle->GetCircle().radius;

    // Line is in world space already (absolute coordinates)
    glm::vec2 lineStart = line->GetLine().start;
    glm::vec2 lineEnd = line->GetLine().end;
    float thickness = line->GetLine().thickness;

    // Find closest point on line to circle center
    glm::vec2 closestPoint = ClosestPointOnLine(circleCenter, lineStart, lineEnd);

    glm::vec2 direction = circleCenter - closestPoint;
    float distance = Distance(circleCenter, closestPoint);
    float totalRadius = radius + thickness;

    if (distance < totalRadius) {
        // Collision detected
        float penetration = totalRadius - distance;
        glm::vec2 normal = (distance > 0.001f) ? Normalize(direction) : glm::vec2(0.0f, 1.0f);

        return CollisionInfo(true, closestPoint, normal, penetration);
    }

    return CollisionInfo();
}

CollisionInfo CollisionSystem::CheckCircleAABB(const CollisionComponent* circle, const CollisionComponent* aabb) {
    glm::vec2 circleCenter = circle->GetWorldCenter();
    float radius = circle->GetCircle().radius;

    glm::vec2 aabbCenter = aabb->GetWorldCenter();
    glm::vec2 aabbHalfSize = aabb->GetAABB().size * 0.5f;

    // Find closest point on AABB to circle center
    glm::vec2 closest = glm::vec2(
        std::max(aabbCenter.x - aabbHalfSize.x, std::min(circleCenter.x, aabbCenter.x + aabbHalfSize.x)),
        std::max(aabbCenter.y - aabbHalfSize.y, std::min(circleCenter.y, aabbCenter.y + aabbHalfSize.y))
    );

    glm::vec2 direction = circleCenter - closest;
    float distance = Distance(circleCenter, closest);

    if (distance < radius) {
        // Collision detected
        float penetration = radius - distance;
        glm::vec2 normal = (distance > 0.001f) ? Normalize(direction) : glm::vec2(0.0f, 1.0f);

        return CollisionInfo(true, closest, normal, penetration);
    }

    return CollisionInfo();
}

CollisionInfo CollisionSystem::CheckAABBAABB(const CollisionComponent* aabbA, const CollisionComponent* aabbB) {
    glm::vec2 centerA = aabbA->GetWorldCenter();
    glm::vec2 centerB = aabbB->GetWorldCenter();
    glm::vec2 halfSizeA = aabbA->GetAABB().size * 0.5f;
    glm::vec2 halfSizeB = aabbB->GetAABB().size * 0.5f;

    glm::vec2 distance = centerB - centerA;
    glm::vec2 combinedHalfSize = halfSizeA + halfSizeB;

    // Check overlap on both axes
    if (std::abs(distance.x) < combinedHalfSize.x && std::abs(distance.y) < combinedHalfSize.y) {
        // Collision detected - find minimum separation axis
        float penetrationX = combinedHalfSize.x - std::abs(distance.x);
        float penetrationY = combinedHalfSize.y - std::abs(distance.y);

        glm::vec2 normal;
        float penetration;
        glm::vec2 contactPoint;

        if (penetrationX < penetrationY) {
            // Separate on X axis
            penetration = penetrationX;
            normal = glm::vec2((distance.x > 0) ? 1.0f : -1.0f, 0.0f);
            contactPoint = centerA + glm::vec2((distance.x > 0) ? halfSizeA.x : -halfSizeA.x, 0.0f);
        } else {
            // Separate on Y axis
            penetration = penetrationY;
            normal = glm::vec2(0.0f, (distance.y > 0) ? 1.0f : -1.0f);
            contactPoint = centerA + glm::vec2(0.0f, (distance.y > 0) ? halfSizeA.y : -halfSizeA.y);
        }

        return CollisionInfo(true, contactPoint, normal, penetration);
    }

    return CollisionInfo();
}

void CollisionSystem::ResolveCollision(const CollisionInfo& collision, CollisionComponent* colliderA, CollisionComponent* colliderB) {
    if (!collision.hasCollision) return;

    Entity* entityA = colliderA->GetOwner();
    Entity* entityB = colliderB->GetOwner();

    if (!entityA || !entityB) return;

    auto transformA = entityA->GetComponent<TransformComponent>();
    auto transformB = entityB->GetComponent<TransformComponent>();
    auto physicsA = entityA->GetComponent<SimplePhysicsComponent>();
    auto physicsB = entityB->GetComponent<SimplePhysicsComponent>();

    if (!transformA || !transformB) return;

    // Position correction to prevent sinking
    float correctionPercent = 0.8f; // How much to correct
    float slop = 0.01f; // Allow small penetration to prevent jitter

    if (collision.penetration > slop) {
        float correctionMagnitude = (collision.penetration - slop) * correctionPercent;
        glm::vec2 correction = collision.normal * correctionMagnitude;

        // Apply position correction based on whether objects are static
        if (!colliderA->IsStatic() && !colliderB->IsStatic()) {
            // Both dynamic - split correction
            glm::vec3 posA = transformA->GetPosition();
            glm::vec3 posB = transformB->GetPosition();

            posA.x -= correction.x * 0.5f;
            posA.y -= correction.y * 0.5f;
            posB.x += correction.x * 0.5f;
            posB.y += correction.y * 0.5f;

            transformA->SetPosition(posA);
            transformB->SetPosition(posB);
        } else if (!colliderA->IsStatic()) {
            // Only A is dynamic
            glm::vec3 posA = transformA->GetPosition();
            posA.x -= correction.x;
            posA.y -= correction.y;
            transformA->SetPosition(posA);
        } else if (!colliderB->IsStatic()) {
            // Only B is dynamic
            glm::vec3 posB = transformB->GetPosition();
            posB.x += correction.x;
            posB.y += correction.y;
            transformB->SetPosition(posB);
        }
    }

    // Velocity resolution for physics objects
    if (physicsA && physicsB) {
        glm::vec2 velA = glm::vec2(physicsA->GetVelocity().x, physicsA->GetVelocity().y);
        glm::vec2 velB = glm::vec2(physicsB->GetVelocity().x, physicsB->GetVelocity().y);

        glm::vec2 relativeVel = velB - velA;
        float velocityAlongNormal = Dot(relativeVel, collision.normal);

        // Don't resolve if velocities are separating
        if (velocityAlongNormal > 0) return;

        // Calculate restitution (bounciness)
        float restitution = std::min(physicsA->GetBounceDamping(), physicsB->GetBounceDamping());

        // Calculate impulse scalar
        float impulseScalar = -(1 + restitution) * velocityAlongNormal;

        // Apply mass weighting if both objects are dynamic
        if (!colliderA->IsStatic() && !colliderB->IsStatic()) {
            float massA = physicsA->GetMass();
            float massB = physicsB->GetMass();
            impulseScalar /= (massA + massB);

            glm::vec2 impulse = collision.normal * impulseScalar;

            velA -= impulse * massB;
            velB += impulse * massA;
        } else if (!colliderA->IsStatic()) {
            // Only A is dynamic - B is static
            velA = velA - collision.normal * 2.0f * velocityAlongNormal * restitution;
        } else if (!colliderB->IsStatic()) {
            // Only B is dynamic - A is static
            velB = velB + collision.normal * 2.0f * velocityAlongNormal * restitution;
        }

        // Update physics velocities
        if (!colliderA->IsStatic()) {
            physicsA->SetVelocity(glm::vec3(velA.x, velA.y, physicsA->GetVelocity().z));
        }
        if (!colliderB->IsStatic()) {
            physicsB->SetVelocity(glm::vec3(velB.x, velB.y, physicsB->GetVelocity().z));
        }
    }
}

// Utility functions
float CollisionSystem::Distance(const glm::vec2& a, const glm::vec2& b) {
    return glm::length(b - a);
}

glm::vec2 CollisionSystem::Normalize(const glm::vec2& v) {
    float length = glm::length(v);
    return (length > 0.001f) ? v / length : glm::vec2(0.0f);
}

float CollisionSystem::Dot(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.x + a.y * b.y;
}

glm::vec2 CollisionSystem::ClosestPointOnLine(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd) {
    glm::vec2 line = lineEnd - lineStart;
    float lineLength = glm::length(line);

    if (lineLength < 0.001f) {
        return lineStart; // Line has no length
    }

    glm::vec2 lineDirection = line / lineLength;
    glm::vec2 toPoint = point - lineStart;

    float projectionLength = Dot(toPoint, lineDirection);
    projectionLength = std::max(0.0f, std::min(lineLength, projectionLength));

    return lineStart + lineDirection * projectionLength;
}

std::string CollisionSystem::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "=== Collision System ===\n";
    oss << "Registered Entities: " << entities.size() << "\n";

    int activeColliders = 0;
    int totalCollisions = 0;

    for (const auto& entity : entities) {
        if (entity && entity->IsActive()) {
            activeColliders++;
            auto collider = entity->GetComponent<CollisionComponent>();
            if (collider) {
                totalCollisions += collider->GetCollisions().size();
            }
        }
    }

    oss << "Active Colliders: " << activeColliders << "\n";
    oss << "Total Collisions: " << totalCollisions;

    return oss.str();
}

} // namespace Logic
} // namespace Engine
