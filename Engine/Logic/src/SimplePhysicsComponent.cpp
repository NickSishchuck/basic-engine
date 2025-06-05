#include "../include/SimplePhysicsComponent.h"
#include "../include/Entity.h"
#include "../include/TransformComponent.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace Engine {
namespace Logic {

void SimplePhysicsComponent::Update(float deltaTime) {
    if (!owner) return;

    auto transform = owner->GetComponent<TransformComponent>();
    if (!transform) return;

    glm::vec3 position = transform->GetPosition();

    // Apply gravity if enabled
    if (affectedByGravity) {
        velocity += acceleration * deltaTime;
    }

    // Update position
    position += velocity * deltaTime;

    // Simple floor collision (bounce)
    if (position.y < 0.5f && velocity.y < 0.0f) { // Assuming cube size ~1.0
        position.y = 0.5f; // Keep above floor
        velocity.y = -velocity.y * bounceDamping; // Bounce with energy loss

        // Add some friction to horizontal movement
        velocity.x *= 0.9f;
        velocity.z *= 0.9f;

        // Stop tiny bounces
        if (std::abs(velocity.y) < 0.1f) {
            velocity.y = 0.0f;
        }
    }

    // Simple boundary checks (keep objects in reasonable bounds)
    if (position.x > 10.0f || position.x < -10.0f) {
        velocity.x = -velocity.x * 0.5f; // Bounce off sides
        position.x = std::clamp(position.x, -10.0f, 10.0f);
    }
    if (position.z > 10.0f || position.z < -10.0f) {
        velocity.z = -velocity.z * 0.5f; // Bounce off front/back
        position.z = std::clamp(position.z, -10.0f, 10.0f);
    }

    transform->SetPosition(position);
}

std::string SimplePhysicsComponent::GetDebugInfo() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n";
    oss << "Mass: " << mass << "\n";
    oss << "Gravity: " << (affectedByGravity ? "ON" : "OFF") << "\n";
    oss << "Bounce Damping: " << bounceDamping;
    return oss.str();
}

} // namespace Logic
} // namespace Engine
