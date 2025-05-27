//TODO: Move scene files into a separate folder
#include "../include/PhysicsTestScene.h"
#include <iostream>
#include <random>
#include <iomanip>
#include <algorithm>

namespace Engine {
namespace Logic {

// SimplePhysicsComponent implementation
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

// PhysicsTestScene implementation
PhysicsTestScene::PhysicsTestScene() {
    scene = std::make_shared<Scene>("Physics Test Scene");
    Initialize();
}

void PhysicsTestScene::Initialize() {
    std::cout << "Initializing Physics Test Scene..." << std::endl;

    // 1. Bouncing ball (high bounce)
    bouncingBall = scene->CreateEntity("Bouncing Ball");
    auto ballTransform = bouncingBall->AddComponent<TransformComponent>(
        glm::vec3(0.0f, 5.0f, -3.0f),  // Start high up
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.4f, 0.4f, 0.4f)    // Smaller than other cubes
    );
    auto ballRender = bouncingBall->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(1.0f, 0.3f, 0.3f),   // Red
        true
    );
    auto ballPhysics = bouncingBall->AddComponent<SimplePhysicsComponent>(0.5f, true);
    ballPhysics->SetBounceDamping(0.9f); // High bounce

    // 2. Floating cube (no gravity)
    floatingCube = scene->CreateEntity("Floating Cube");
    auto floatTransform = floatingCube->AddComponent<TransformComponent>(
        glm::vec3(3.0f, 3.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.6f, 0.6f, 0.6f)
    );
    auto floatRender = floatingCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(0.3f, 0.3f, 1.0f),   // Blue
        true
    );
    auto floatPhysics = floatingCube->AddComponent<SimplePhysicsComponent>(1.0f, false); // No gravity
    floatPhysics->SetVelocity(glm::vec3(1.0f, 0.5f, 0.0f)); // Initial drift

    // 3. Heavy cube (falls fast, low bounce)
    auto heavyCube = scene->CreateEntity("Heavy Cube");
    auto heavyTransform = heavyCube->AddComponent<TransformComponent>(
        glm::vec3(-2.0f, 6.0f, -4.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.8f, 0.8f, 0.8f)
    );
    auto heavyRender = heavyCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(0.5f, 0.5f, 0.5f),   // Gray
        true
    );
    auto heavyPhysics = heavyCube->AddComponent<SimplePhysicsComponent>(5.0f, true); // Heavy
    heavyPhysics->SetBounceDamping(0.3f); // Low bounce

    // 4. Light cube (affected more by forces)
    auto lightCube = scene->CreateEntity("Light Cube");
    auto lightTransform = lightCube->AddComponent<TransformComponent>(
        glm::vec3(1.5f, 4.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.5f, 0.5f, 0.5f)
    );
    auto lightRender = lightCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(1.0f, 1.0f, 0.3f),   // Yellow
        true
    );
    auto lightPhysics = lightCube->AddComponent<SimplePhysicsComponent>(0.2f, true); // Light
    lightPhysics->SetBounceDamping(0.8f);
    lightPhysics->SetVelocity(glm::vec3(2.0f, 0.0f, 1.0f)); // Initial sideways motion

    std::cout << "Physics Test Scene initialized with " << scene->GetEntityCount() << " entities" << std::endl;
}

void PhysicsTestScene::Update(float deltaTime) {
    if (!physicsEnabled) {
        // Still update the scene but don't apply physics time scaling
        scene->Update(0.0f);
        return;
    }

    // Apply time scaling to physics
    float scaledDeltaTime = deltaTime * timeScale;

    // Update the base scene
    scene->Update(scaledDeltaTime);
}

void PhysicsTestScene::Reset() {
    std::cout << "Resetting Physics Test Scene..." << std::endl;

    // Clear the falling cubes vector first to remove any shared_ptr references
    fallingCubes.clear();

    // Collect IDs of random cubes to remove
    std::vector<int> randomCubeIds;
    for (const auto& entity : scene->GetEntities()) {
        if (!entity) continue;

        const std::string& name = entity->GetName();
        if (name.find("Random Cube") == 0) {
            randomCubeIds.push_back(entity->GetID());
        }
    }

    // Remove all random cubes from the scene
    for (int cubeId : randomCubeIds) {
        scene->RemoveEntity(cubeId);
    }

    std::cout << "Removed " << randomCubeIds.size() << " random cubes" << std::endl;

    // Reset all remaining entities to their initial states
    for (const auto& entity : scene->GetEntities()) {
        if (!entity) continue;

        // Reset physics velocity
        auto physics = entity->GetComponent<SimplePhysicsComponent>();
        if (physics) {
            physics->SetVelocity(glm::vec3(0.0f));
        }

        // Reset transform position
        auto transform = entity->GetComponent<TransformComponent>();
        if (!transform) continue;

        const std::string& name = entity->GetName();
        if (name == "Bouncing Ball") {
            transform->SetPosition(glm::vec3(0.0f, 5.0f, -3.0f));
        } else if (name == "Floating Cube") {
            transform->SetPosition(glm::vec3(3.0f, 3.0f, -2.0f));
            if (physics) physics->SetVelocity(glm::vec3(1.0f, 0.5f, 0.0f));
        } else if (name == "Heavy Cube") {
            transform->SetPosition(glm::vec3(-2.0f, 6.0f, -4.0f));
        } else if (name == "Light Cube") {
            transform->SetPosition(glm::vec3(1.5f, 4.0f, -1.0f));
            if (physics) physics->SetVelocity(glm::vec3(2.0f, 0.0f, 1.0f));
        }
    }
}

void PhysicsTestScene::SpawnRandomCube() {
    static int cubeCounter = 0;
    cubeCounter++;

    // Random generator
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-3.0f, 3.0f);
    std::uniform_real_distribution<float> heightDist(3.0f, 8.0f);
    std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);
    std::uniform_real_distribution<float> massDist(0.5f, 3.0f);
    std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);

    auto newCube = scene->CreateEntity("Random Cube " + std::to_string(cubeCounter));
    auto transform = newCube->AddComponent<TransformComponent>(
        glm::vec3(posDist(gen), heightDist(gen), posDist(gen)),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.5f, 0.5f, 0.5f)
    );
    auto render = newCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen)),
        true
    );
    auto physics = newCube->AddComponent<SimplePhysicsComponent>(massDist(gen), true);
    physics->SetVelocity(glm::vec3(velDist(gen), 0.0f, velDist(gen)));

    fallingCubes.push_back(newCube);

    std::cout << "Spawned random cube: " << newCube->GetName() << std::endl;
}

void PhysicsTestScene::Destroy() {
    if (scene) {
        scene->Destroy();
    }
    fallingCubes.clear();
    bouncingBall = nullptr;
    floatingCube = nullptr;
    pendulum = nullptr;
}

} // namespace Logic
} // namespace Engine
