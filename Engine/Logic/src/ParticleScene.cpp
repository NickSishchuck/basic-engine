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

        // BETTER CUP DIMENSIONS for visibility
        cupWidth = 100.0f;           // Narrower cup width
        cupHeight = 150.0f;          // Shorter height
        wallThickness = 8.0f;        // Thinner walls
        cupCenter = glm::vec2(0.0f, 0.0f); // Move cup higher up

        // Particle spawning
        particleRadius = 8.0f;
        minParticleRadius = 3.0f;
        maxParticleRadius = 15.0f;
        spawnArea = glm::vec2(140.0f, 30.0f); // Adjust to new cup width
        spawnHeight = 80.0f;         // Lower spawn height

        Initialize();
    }

    void ParticleScene::Initialize() {
        std::cout << "Initializing Particle Scene..." << std::endl;
        std::cout << "Cup parameters:" << std::endl;
        std::cout << "  Center: (" << cupCenter.x << ", " << cupCenter.y << ")" << std::endl;
        std::cout << "  Width: " << cupWidth << ", Height: " << cupHeight << std::endl;
        std::cout << "  Spawn height: " << spawnHeight << std::endl;

        // Create the cup boundaries
        CreateCupBoundaries();

        // Spawn initial particles at known positions for testing
        std::cout << "Spawning initial test particles..." << std::endl;

        // Spawn one particle at a known good position for testing
        glm::vec2 testPos1 = glm::vec2(0.0f, cupCenter.y + cupHeight + 50.0f); // Center above cup
        SpawnParticle(testPos1, 12.0f); // Larger particle
        std::cout << "Test particle 1 spawned at (" << testPos1.x << ", " << testPos1.y << ")" << std::endl;

        // Spawn a few more at different positions
        glm::vec2 testPos2 = glm::vec2(-30.0f, cupCenter.y + cupHeight + 80.0f);
        SpawnParticle(testPos2, 10.0f);
        std::cout << "Test particle 2 spawned at (" << testPos2.x << ", " << testPos2.y << ")" << std::endl;

        glm::vec2 testPos3 = glm::vec2(30.0f, cupCenter.y + cupHeight + 60.0f);
        SpawnParticle(testPos3, 8.0f);
        std::cout << "Test particle 3 spawned at (" << testPos3.x << ", " << testPos3.y << ")" << std::endl;

        std::cout << "Particle Scene initialized with cup boundaries and " << particles.size() << " test particles" << std::endl;
        std::cout << "Expected particle positions:" << std::endl;
        for (size_t i = 0; i < particles.size(); ++i) {
            auto transform = particles[i]->GetComponent<TransformComponent>();
            if (transform) {
                glm::vec3 pos = transform->GetPosition();
                std::cout << "  Particle " << (i+1) << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            }
        }
    }


void ParticleScene::CreateCupBoundaries() {
    // Calculate wall positions for a proper U-shaped cup
    float halfWidth = cupWidth / 2.0f;
    float cupBottom = cupCenter.y;          // Bottom of the cup
    float cupTop = cupCenter.y + cupHeight; // Top of the cup

    std::cout << "Creating cup boundaries:" << std::endl;
    std::cout << "  Cup center: (" << cupCenter.x << ", " << cupCenter.y << ")" << std::endl;
    std::cout << "  Cup width: " << cupWidth << ", height: " << cupHeight << std::endl;
    std::cout << "  Cup bottom Y: " << cupBottom << ", top Y: " << cupTop << std::endl;

    // BOTTOM WALL - horizontal line at the bottom of the cup
    glm::vec2 bottomStart = glm::vec2(cupCenter.x - halfWidth, cupBottom);
    glm::vec2 bottomEnd = glm::vec2(cupCenter.x + halfWidth, cupBottom);
    CreateWall(bottomWall, "Bottom Wall", bottomStart, bottomEnd, glm::vec3(1.0f, 0.0f, 0.0f)); // Red

    std::cout << "  Bottom wall: (" << bottomStart.x << ", " << bottomStart.y
              << ") to (" << bottomEnd.x << ", " << bottomEnd.y << ")" << std::endl;

    // LEFT WALL - vertical line on the left side
    glm::vec2 leftStart = glm::vec2(cupCenter.x - halfWidth, cupBottom);
    glm::vec2 leftEnd = glm::vec2(cupCenter.x - halfWidth, cupTop);
    CreateWall(leftWall, "Left Wall", leftStart, leftEnd, glm::vec3(0.0f, 1.0f, 0.0f)); // Green

    std::cout << "  Left wall: (" << leftStart.x << ", " << leftStart.y
              << ") to (" << leftEnd.x << ", " << leftEnd.y << ")" << std::endl;

    // RIGHT WALL - vertical line on the right side
    glm::vec2 rightStart = glm::vec2(cupCenter.x + halfWidth, cupBottom);
    glm::vec2 rightEnd = glm::vec2(cupCenter.x + halfWidth, cupTop);
    CreateWall(rightWall, "Right Wall", rightStart, rightEnd, glm::vec3(0.0f, 0.0f, 1.0f)); // Blue

    std::cout << "  Right wall: (" << rightStart.x << ", " << rightStart.y
              << ") to (" << rightEnd.x << ", " << rightEnd.y << ")" << std::endl;
}

void ParticleScene::CreateWall(std::shared_ptr<Entity>& wall, const std::string& name,
                              const glm::vec2& start, const glm::vec2& end, const glm::vec3& color) {
    wall = scene->CreateEntity(name);

    // Calculate wall properties
    glm::vec2 wallCenter = (start + end) * 0.5f;
    glm::vec2 wallDirection = end - start;
    float wallLength = glm::length(wallDirection);

    // Determine if this is a horizontal or vertical wall
    bool isHorizontal = std::abs(wallDirection.x) > std::abs(wallDirection.y);

    glm::vec2 wallSize;
    if (isHorizontal) {
        // Horizontal wall (like bottom wall)
        wallSize = glm::vec2(wallLength, wallThickness);
    } else {
        // Vertical wall (like left/right walls)
        wallSize = glm::vec2(wallThickness, wallLength);
    }

    std::cout << "Creating wall '" << name << "':" << std::endl;
    std::cout << "  Line: (" << start.x << ", " << start.y << ") to (" << end.x << ", " << end.y << ")" << std::endl;
    std::cout << "  Center: (" << wallCenter.x << ", " << wallCenter.y << ")" << std::endl;
    std::cout << "  Size: " << wallSize.x << " x " << wallSize.y << std::endl;
    std::cout << "  Type: " << (isHorizontal ? "Horizontal" : "Vertical") << std::endl;

    // Transform component - position at wall center
    auto transform = wall->AddComponent<TransformComponent>(
        glm::vec3(wallCenter.x, wallCenter.y, 0.0f), // Position at calculated center
        glm::vec3(0.0f, 0.0f, 0.0f),                // No rotation
        glm::vec3(wallSize.x, wallSize.y, wallThickness) // Scale to wall size
    );

    // Render component - visual representation
    auto render = wall->AddComponent<RenderComponent>(
        PrimitiveType::CUBE, // Render as rectangle
        color,               // Wall color
        true                 // Visible
    );

    // Collision component - line collider for precise physics
    auto collision = wall->AddComponent<CollisionComponent>(
        CollisionShape::LINE_SEGMENT,
        false, // Not a trigger
        true,  // Static object
        "wall"
    );
    collision->SetLine(start, end, wallThickness / 2.0f); // Use half thickness for collision

    // Register with collision system
    collisionSystem->RegisterEntity(wall);

    std::cout << "  Wall '" << name << "' created successfully!" << std::endl;
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
    // Spawn particles above the cup, within the cup width
    float spawnX = cupCenter.x + (positionDist(gen) * (cupWidth * 0.8f) / 2.0f); // Slightly narrower than cup
    float spawnY = cupCenter.y + cupHeight + spawnHeight + (positionDist(gen) * 20.0f); // Above cup with some randomness

    std::cout << "Generated spawn position: (" << spawnX << ", " << spawnY << ")" << std::endl;
    std::cout << "  Cup range: X(" << (cupCenter.x - cupWidth/2) << " to " << (cupCenter.x + cupWidth/2)
              << ") Y(" << cupCenter.y << " to " << (cupCenter.y + cupHeight) << ")" << std::endl;

    return glm::vec2(spawnX, spawnY);
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
