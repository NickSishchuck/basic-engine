#include "../include/ParticleScene.h"
#include "../include/SimplePhysicsComponent.h"
#include <iostream>
#include <algorithm>

namespace Engine {
namespace Logic {

ParticleScene::ParticleScene()
    : gen(rd()), colorDist(0.3f, 1.0f), positionDist(-1.0f, 1.0f), velocityDist(-50.0f, 50.0f) {
    scene = std::make_shared<Scene>("Particle Physics Scene");
    collisionSystem = std::make_unique<CollisionSystem>();
    Initialize();
}

void ParticleScene::Initialize() {
    std::cout << "Initializing Particle Scene..." << std::endl;

    // Create the cup boundaries
    CreateCupBoundaries();

    std::cout << "Particle Scene initialized with cup boundaries" << std::endl;
    std::cout << "Cup dimensions: " << cupWidth << "x" << cupHeight << " pixels" << std::endl;
}

void ParticleScene::CreateCupBoundaries() {
    // Calculate wall positions based on cup dimensions
    float halfWidth = cupWidth / 2.0f;

    // Bottom wall (full width)
    glm::vec2 bottomStart = cupCenter + glm::vec2(-halfWidth - wallThickness, 0.0f);
    glm::vec2 bottomEnd = cupCenter + glm::vec2(halfWidth + wallThickness, 0.0f);
    CreateWall(bottomWall, "Bottom Wall", bottomStart, bottomEnd, glm::vec3(0.6f, 0.6f, 0.6f));

    // Left wall (vertical)
    glm::vec2 leftStart = cupCenter + glm::vec2(-halfWidth, 0.0f);
    glm::vec2 leftEnd = cupCenter + glm::vec2(-halfWidth, cupHeight);
    CreateWall(leftWall, "Left Wall", leftStart, leftEnd, glm::vec3(0.7f, 0.4f, 0.4f));

    // Right wall (vertical)
    glm::vec2 rightStart = cupCenter + glm::vec2(halfWidth, 0.0f);
    glm::vec2 rightEnd = cupCenter + glm::vec2(halfWidth, cupHeight);
    CreateWall(rightWall, "Right Wall", rightStart, rightEnd, glm::vec3(0.4f, 0.7f, 0.4f));
}

void ParticleScene::CreateWall(std::shared_ptr<Entity>& wall, const std::string& name,
                              const glm::vec2& start, const glm::vec2& end, const glm::vec3& color) {
    wall = scene->CreateEntity(name);

    // Transform - walls don't need position since line collider uses absolute coordinates
    auto transform = wall->AddComponent<TransformComponent>(
        glm::vec3(0.0f, 0.0f, 0.0f), // Position (not used for line colliders)
        glm::vec3(0.0f, 0.0f, 0.0f), // Rotation
        glm::vec3(1.0f, 1.0f, 1.0f)  // Scale
    );

    // Render component - for visual representation, create a rectangle
    glm::vec2 wallCenter = (start + end) * 0.5f;
    glm::vec2 wallSize = glm::vec2(
        std::abs(end.x - start.x) + wallThickness,
        std::abs(end.y - start.y) + wallThickness
    );

    // Adjust transform position to wall center for rendering
    transform->SetPosition(glm::vec3(wallCenter.x, wallCenter.y, 0.0f));

    auto render = wall->AddComponent<RenderComponent>(
        PrimitiveType::CUBE, // We'll render as rectangles for now
        color,
        true
    );

    // Set scale to match wall size
    transform->SetScale(glm::vec3(wallSize.x, wallSize.y, wallThickness));

    // Collision component - line collider for precise collision detection
    auto collision = wall->AddComponent<CollisionComponent>(
        CollisionShape::LINE_SEGMENT,
        false, // Not a trigger
        true,  // Static object
        "wall"
    );
    collision->SetLine(start, end, wallThickness);

    // Register with collision system
    collisionSystem->RegisterEntity(wall);
}

void ParticleScene::Update(float deltaTime) {
    if (!physicsEnabled) {
        // Still update the scene but don't apply physics time scaling
        scene->Update(0.0f);
        return;
    }

    // Apply time scaling to physics
    float scaledDeltaTime = deltaTime * timeScale;

    // Update physics for all particles
    UpdatePhysics(scaledDeltaTime);

    // Update collision system
    collisionSystem->Update(scaledDeltaTime);

    // Update the base scene
    scene->Update(scaledDeltaTime);

    // Clean up any destroyed particles
    CleanupDestroyedParticles();
}

void ParticleScene::UpdatePhysics(float deltaTime) {
    // Apply gravity and update positions for all particles
    for (auto& particle : particles) {
        if (!particle || !particle->IsActive()) continue;

        auto physics = particle->GetComponent<SimplePhysicsComponent>();
        if (physics) {
            // Apply 2D gravity
            glm::vec3 currentVel = physics->GetVelocity();
            currentVel.x += gravity.x * deltaTime;
            currentVel.y += gravity.y * deltaTime;

            // Simple air resistance for more realistic movement
            float airResistance = 0.99f;
            currentVel.x *= airResistance;
            currentVel.z = 0.0f; // Keep Z velocity at 0 for 2D simulation

            physics->SetVelocity(currentVel);
        }
    }
}

void ParticleScene::SpawnParticle() {
    glm::vec2 spawnPos = GenerateSpawnPosition();
    SpawnParticle(spawnPos, particleRadius);
}

void ParticleScene::SpawnParticle(const glm::vec2& position, float radius) {
    if (radius < 0) radius = particleRadius;

    static int particleCounter = 0;
    particleCounter++;

    auto particle = scene->CreateEntity("Particle_" + std::to_string(particleCounter));

    // Transform component
    auto transform = particle->AddComponent<TransformComponent>(
        glm::vec3(position.x, position.y, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(radius * 2.0f, radius * 2.0f, radius * 2.0f) // Scale for visual size
    );

    // Render component - circle
    auto render = particle->AddComponent<RenderComponent>(
        PrimitiveType::CIRCLE,
        GenerateParticleColor(),
        true
    );

    // Collision component - circle collider
    auto collision = particle->AddComponent<CollisionComponent>(
        CollisionShape::CIRCLE,
        false, // Not a trigger
        false, // Dynamic object
        "particle"
    );
    collision->SetCircle(radius, glm::vec2(0.0f, 0.0f));

    // Physics component
    auto physics = particle->AddComponent<SimplePhysicsComponent>(
        particleMass * radius, // Mass proportional to radius
        true // Affected by gravity
    );
    physics->SetBounceDamping(particleBounciness);
    physics->SetVelocity(glm::vec3(GenerateInitialVelocity(), 0.0f));

    // Add to particle list and collision system
    particles.push_back(particle);
    collisionSystem->RegisterEntity(particle);

    std::cout << "Spawned particle at (" << position.x << ", " << position.y
              << ") with radius " << radius << std::endl;
}

glm::vec3 ParticleScene::GenerateParticleColor() {
    return glm::vec3(
        colorDist(gen),
        colorDist(gen),
        colorDist(gen)
    );
}

glm::vec2 ParticleScene::GenerateSpawnPosition() {
    float x = cupCenter.x + (positionDist(gen) * spawnArea.x / 2.0f);
    float y = cupCenter.y + cupHeight + spawnHeight + (positionDist(gen) * spawnArea.y / 2.0f);
    return glm::vec2(x, y);
}

glm::vec2 ParticleScene::GenerateInitialVelocity() {
    return glm::vec2(
        velocityDist(gen) * 0.5f, // Small horizontal velocity
        velocityDist(gen) * 0.2f  // Small vertical velocity
    );
}

void ParticleScene::CleanupDestroyedParticles() {
    // Remove inactive particles from our list
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [this](const std::shared_ptr<Entity>& particle) {
                if (!particle || !particle->IsActive()) {
                    // Unregister from collision system
                    if (particle) {
                        collisionSystem->UnregisterEntity(particle);
                    }
                    return true;
                }
                return false;
            }),
        particles.end()
    );
}

void ParticleScene::SetParticleRadius(float radius) {
    particleRadius = std::clamp(radius, minParticleRadius, maxParticleRadius);
}

void ParticleScene::SetCupDimensions(float width, float height) {
    cupWidth = width;
    cupHeight = height;

    // Recreate boundaries
    if (leftWall) collisionSystem->UnregisterEntity(leftWall);
    if (rightWall) collisionSystem->UnregisterEntity(rightWall);
    if (bottomWall) collisionSystem->UnregisterEntity(bottomWall);

    scene->RemoveEntity(leftWall);
    scene->RemoveEntity(rightWall);
    scene->RemoveEntity(bottomWall);

    CreateCupBoundaries();
}

void ParticleScene::ClearAllParticles() {
    // Unregister all particles from collision system
    for (auto& particle : particles) {
        if (particle) {
            collisionSystem->UnregisterEntity(particle);
            scene->RemoveEntity(particle);
        }
    }

    particles.clear();
    std::cout << "Cleared all particles" << std::endl;
}

void ParticleScene::Reset() {
    std::cout << "Resetting Particle Scene..." << std::endl;
    ClearAllParticles();
}

void ParticleScene::Destroy() {
    if (scene) {
        scene->Destroy();
    }

    if (collisionSystem) {
        collisionSystem->Clear();
    }

    particles.clear();
    leftWall = nullptr;
    rightWall = nullptr;
    bottomWall = nullptr;
}

} // namespace Logic
} // namespace Engine
