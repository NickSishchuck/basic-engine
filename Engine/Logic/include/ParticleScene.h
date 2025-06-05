#ifndef PARTICLE_SCENE_H
#define PARTICLE_SCENE_H

#include "Scene.h"
#include "CollisionSystem.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "CollisionComponent.h"
#include <memory>
#include <vector>
#include <random>

namespace Engine {
namespace Logic {

class ParticleScene {
private:
    std::shared_ptr<Scene> scene;
    std::unique_ptr<CollisionSystem> collisionSystem;

    // Boundary entities (the "cup")
    std::shared_ptr<Entity> leftWall;
    std::shared_ptr<Entity> rightWall;
    std::shared_ptr<Entity> bottomWall;

    // Particle management
    std::vector<std::shared_ptr<Entity>> particles;

    // Scene parameters
    float cupWidth = 200.0f;           // Width of the cup opening
    float cupHeight = 300.0f;          // Height of the cup
    float wallThickness = 10.0f;       // Thickness of walls
    glm::vec2 cupCenter = glm::vec2(0.0f, -150.0f); // Bottom center of cup

    // Particle spawning
    float particleRadius = 8.0f;       // Current particle size
    float minParticleRadius = 3.0f;    // Minimum particle size
    float maxParticleRadius = 15.0f;   // Maximum particle size
    glm::vec2 spawnArea = glm::vec2(180.0f, 50.0f); // Width and height of spawn area
    float spawnHeight = 100.0f;        // Height above cup to spawn particles

    // Particle physics settings
    float particleBounciness = 0.6f;   // How bouncy particles are
    float particleMass = 1.0f;         // Base mass for particles
    glm::vec2 gravity = glm::vec2(0.0f, -500.0f); // 2D gravity

    // Random generation
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> colorDist;
    std::uniform_real_distribution<float> positionDist;
    std::uniform_real_distribution<float> velocityDist;

    // Scene state
    bool physicsEnabled = true;
    float timeScale = 1.0f;

public:
    ParticleScene();
    ~ParticleScene() = default;

    void Initialize();
    void Update(float deltaTime);
    void Destroy();
    void Reset(); // Clear all particles

    // Access to the underlying scene
    std::shared_ptr<Scene> GetScene() const { return scene; }

    // Particle spawning
    void SpawnParticle();
    void SpawnParticle(const glm::vec2& position, float radius = -1.0f);

    // Scene controls
    void SetParticleRadius(float radius);
    float GetParticleRadius() const { return particleRadius; }
    float GetMinParticleRadius() const { return minParticleRadius; }
    float GetMaxParticleRadius() const { return maxParticleRadius; }

    void SetPhysicsEnabled(bool enabled) { physicsEnabled = enabled; }
    bool IsPhysicsEnabled() const { return physicsEnabled; }

    void SetTimeScale(float scale) { timeScale = scale; }
    float GetTimeScale() const { return timeScale; }

    void SetParticleBounciness(float bounce) { particleBounciness = bounce; }
    float GetParticleBounciness() const { return particleBounciness; }

    // Cup configuration
    void SetCupDimensions(float width, float height);
    float GetCupWidth() const { return cupWidth; }
    float GetCupHeight() const { return cupHeight; }

    // Particle management
    size_t GetParticleCount() const { return particles.size(); }
    void ClearAllParticles();

    // Collision system access
    CollisionSystem* GetCollisionSystem() const { return collisionSystem.get(); }

private:
    void CreateCupBoundaries();
    void CreateWall(std::shared_ptr<Entity>& wall, const std::string& name,
                   const glm::vec2& start, const glm::vec2& end, const glm::vec3& color);

    glm::vec3 GenerateParticleColor();
    glm::vec2 GenerateSpawnPosition();
    glm::vec2 GenerateInitialVelocity();

    void UpdatePhysics(float deltaTime);
    void CleanupDestroyedParticles();
};

} // namespace Logic
} // namespace Engine

#endif
