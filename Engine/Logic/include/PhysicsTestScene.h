//TODO: Move scene files into a separate folder
#ifndef PHYSICS_TEST_SCENE_H
#define PHYSICS_TEST_SCENE_H

#include "Scene.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include <memory>
#include <vector>

namespace Engine {
namespace Logic {

// Simple physics component for testing
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

class PhysicsTestScene {
private:
    std::shared_ptr<Scene> scene;

    // Physics entities
    std::vector<std::shared_ptr<Entity>> fallingCubes;
    std::shared_ptr<Entity> bouncingBall;
    std::shared_ptr<Entity> floatingCube;
    std::shared_ptr<Entity> pendulum;

    // Scene parameters
    float floorY = 0.0f;
    bool physicsEnabled = true;
    float timeScale = 1.0f;

public:
    PhysicsTestScene();
    ~PhysicsTestScene() = default;

    void Initialize();
    void Update(float deltaTime);
    void Destroy();
    void Reset(); // Reset all physics objects

    // Access to the underlying scene
    std::shared_ptr<Scene> GetScene() const { return scene; }

    // Physics controls
    void SetPhysicsEnabled(bool enabled) { physicsEnabled = enabled; }
    bool IsPhysicsEnabled() const { return physicsEnabled; }

    void SetTimeScale(float scale) { timeScale = scale; }
    float GetTimeScale() const { return timeScale; }

    void SpawnRandomCube(); // Add a new falling cube
};

} // namespace Logic
} // namespace Engine

#endif
