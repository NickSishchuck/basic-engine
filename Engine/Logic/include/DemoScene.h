//TODO: Move scene files into a separate folder
#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include "Scene.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include <memory>

namespace Engine {
namespace Logic {

class DemoScene {
private:
    std::shared_ptr<Scene> scene;

    // References to our animated entities for easy access
    std::shared_ptr<Entity> movingCube;
    std::shared_ptr<Entity> rotatingCube;
    std::shared_ptr<Entity> staticCube;

    // Animation state
    float animationDirection = 1.0f;
    float animationSpeed = 2.0f;

public:
    DemoScene();
    ~DemoScene() = default;

    void Initialize();
    void Update(float deltaTime);
    void Destroy();

    // Access to the underlying scene
    std::shared_ptr<Scene> GetScene() const { return scene; }

    // Demo-specific controls
    void SetAnimationSpeed(float speed) { animationSpeed = speed; }
    float GetAnimationSpeed() const { return animationSpeed; }
};

} // namespace Logic
} // namespace Engine

#endif
