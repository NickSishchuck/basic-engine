//TODO: Move scene files into a separate folder
#include "../include/DemoScene.h"
#include <iostream>

namespace Engine {
namespace Logic {

DemoScene::DemoScene() {
    scene = std::make_shared<Scene>("Demo Scene");
    Initialize();
}

void DemoScene::Initialize() {
    std::cout << "Initializing Demo Scene..." << std::endl;

    // 1. Moving cube (left-right movement)
    movingCube = scene->CreateEntity("Moving Cube");
    auto movingTransform = movingCube->AddComponent<TransformComponent>(
        glm::vec3(0.0f, 1.0f, -3.0f),  // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // rotation
        glm::vec3(0.8f, 0.8f, 0.8f)    // scale
    );
    auto movingRender = movingCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(1.0f, 0.5f, 0.2f),   // orange color
        true
    );

    // 2. Static cube
    staticCube = scene->CreateEntity("Static Cube");
    auto staticTransform = staticCube->AddComponent<TransformComponent>(
        glm::vec3(3.0f, 0.5f, -2.0f),  // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // rotation
        glm::vec3(0.5f, 0.5f, 0.5f)    // scale
    );
    auto staticRender = staticCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(0.2f, 0.8f, 0.2f),   // green color
        true
    );

    // 3. Rotating cube
    rotatingCube = scene->CreateEntity("Rotating Cube");
    auto rotatingTransform = rotatingCube->AddComponent<TransformComponent>(
        glm::vec3(-2.0f, 1.5f, -4.0f), // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // rotation
        glm::vec3(0.6f, 0.6f, 0.6f)    // scale
    );
    auto rotatingRender = rotatingCube->AddComponent<RenderComponent>(
        PrimitiveType::CUBE,
        glm::vec3(0.8f, 0.2f, 0.8f),   // purple color
        true
    );

    std::cout << "Demo Scene initialized with " << scene->GetEntityCount() << " entities" << std::endl;
}

void DemoScene::Update(float deltaTime) {
    // Update the base scene first
    scene->Update(deltaTime);

    // Demo-specific animations
    if (movingCube && movingCube->IsActive()) {
        auto transform = movingCube->GetComponent<TransformComponent>();
        if (transform) {
            glm::vec3 currentPos = transform->GetPosition();
            currentPos.x += animationDirection * animationSpeed * deltaTime;

            // Reverse direction at boundaries
            if (currentPos.x > 2.0f) {
                animationDirection = -1.0f;
                currentPos.x = 2.0f; // Clamp to boundary
            } else if (currentPos.x < -2.0f) {
                animationDirection = 1.0f;
                currentPos.x = -2.0f; // Clamp to boundary
            }

            transform->SetPosition(currentPos);
        }
    }

    // Rotating cube animation
    if (rotatingCube && rotatingCube->IsActive()) {
        auto transform = rotatingCube->GetComponent<TransformComponent>();
        if (transform) {
            glm::vec3 rotation = transform->GetRotation();
            rotation.y += 1.0f * deltaTime; // 1 radian per second around Y
            rotation.x += 0.5f * deltaTime; // 0.5 radians per second around X
            transform->SetRotation(rotation);
        }
    }
}

void DemoScene::Destroy() {
    if (scene) {
        scene->Destroy();
    }
    movingCube = nullptr;
    rotatingCube = nullptr;
    staticCube = nullptr;
}

} // namespace Logic
} // namespace Engine
