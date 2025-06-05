#ifndef SIMPLE_PHYSICS_COMPONENT_H
#define SIMPLE_PHYSICS_COMPONENT_H

#include "Component.h"
#include <glm/glm.hpp>
#include <string>

namespace Engine {
namespace Logic {

// Simple physics component for basic physics simulation
class SimplePhysicsComponent : public Component {
private:
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f, -9.81f, 0.0f); // Gravity
    float mass = 1.0f;
    float bounceDamping = 0.7f; // Energy loss on bounce
    bool affectedByGravity = true;

public:
    SimplePhysicsComponent(float m = 1.0f, bool gravity = true)
        : mass(m), affectedByGravity(gravity) {}

    void Update(float deltaTime) override;
    std::string GetTypeName() const override { return "SimplePhysicsComponent"; }
    std::string GetDebugInfo() const override;

    // Physics properties
    void SetVelocity(const glm::vec3& vel) { velocity = vel; }
    const glm::vec3& GetVelocity() const { return velocity; }
    void AddVelocity(const glm::vec3& vel) { velocity += vel; }

    void SetAcceleration(const glm::vec3& acc) { acceleration = acc; }
    const glm::vec3& GetAcceleration() const { return acceleration; }

    void SetMass(float m) { mass = m; }
    float GetMass() const { return mass; }

    void SetBounceDamping(float damping) { bounceDamping = damping; }
    float GetBounceDamping() const { return bounceDamping; }

    void SetAffectedByGravity(bool affected) { affectedByGravity = affected; }
    bool IsAffectedByGravity() const { return affectedByGravity; }

    // Physics actions
    void ApplyForce(const glm::vec3& force) { velocity += force / mass; }
    void ApplyImpulse(const glm::vec3& impulse) { velocity += impulse; }
};

} // namespace Logic
} // namespace Engine

#endif
