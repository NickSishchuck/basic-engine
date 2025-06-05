#include "../../Common/include/RendererWrapper.h"
#include "../../Logic/include/SceneManager.h"
#include "../../../renderer/include/ImGuiManager.h"
#include "../../Logic/include/DemoScene.h"
#include "../../Logic/include/PhysicsTestScene.h"
#include "../../Logic/include/ParticleScene.h"
#include <iostream>
#include <glm/glm.hpp>

// Application states
enum class AppState {
    MAIN_MENU,
    DEMO_SCENE,
    PHYSICS_SCENE,
    PARTICLE_SCENE,
    EXITING
};

class BasicEngineApp {
private:
    Engine::Common::OpenGLRendererWrapper renderer;
    Engine::Logic::SceneManager sceneManager;

    // Scene instances
    std::unique_ptr<Engine::Logic::DemoScene> demoScene;
    std::unique_ptr<Engine::Logic::PhysicsTestScene> physicsScene;
    std::unique_ptr<Engine::Logic::ParticleScene> particleScene;  // New particle scene

    // Application state
    AppState currentState = AppState::MAIN_MENU;
    AppState nextState = AppState::MAIN_MENU;
    bool stateTransitionPending = false;

    // Menu variables
    bool showMainMenu = true;
    bool showDebugInfo = false;

    // Physics scene controls
    bool physicsEnabled = true;
    float physicsTimeScale = 1.0f;

    // Particle scene controls
    float particleRadius = 8.0f;
    bool particlePhysicsEnabled = true;
    float particleTimeScale = 1.0f;

public:
    bool Initialize() {
        if (!renderer.Initialize(1920, 1080, "BasicEngine - Scene Manager Demo")) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        // Create scene instances
        demoScene = std::make_unique<Engine::Logic::DemoScene>();
        physicsScene = std::make_unique<Engine::Logic::PhysicsTestScene>();
        particleScene = std::make_unique<Engine::Logic::ParticleScene>();  // Create particle scene

        // Register scenes with the scene manager
        sceneManager.RegisterScene("Demo", demoScene->GetScene());
        sceneManager.RegisterScene("Physics Test", physicsScene->GetScene());
        sceneManager.RegisterScene("Particle Physics", particleScene->GetScene());  // Register particle scene

        std::cout << "BasicEngine initialized successfully!" << std::endl;
        sceneManager.PrintSceneList();

        return true;
    }

    void Run() {
        float lastFrameTime = glfwGetTime();

        while (!glfwWindowShouldClose(renderer.GetWindow()) && currentState != AppState::EXITING) {
            // Calculate delta time
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;

            // Handle state transitions
            ProcessStateTransition();

            // Update current state
            Update(deltaTime);

            // Render
            Render();
        }
    }

    void Shutdown() {
        demoScene.reset();
        physicsScene.reset();
        particleScene.reset();  // Clean up particle scene
        renderer.Shutdown();
    }

private:
    void ProcessStateTransition() {
        if (!stateTransitionPending) return;

        // Handle state exit
        switch (currentState) {
            case AppState::DEMO_SCENE:
            case AppState::PHYSICS_SCENE:
            case AppState::PARTICLE_SCENE:  // Handle particle scene exit
                sceneManager.UnloadCurrentScene();
                break;
            default:
                break;
        }

        // Switch to new state
        currentState = nextState;

        // Handle state enter
        switch (currentState) {
            case AppState::MAIN_MENU:
                showMainMenu = true;
                std::cout << "Entered Main Menu" << std::endl;
                break;

            case AppState::DEMO_SCENE:
                sceneManager.LoadScene("Demo");
                showMainMenu = false;
                std::cout << "Entered Demo Scene" << std::endl;
                break;

            case AppState::PHYSICS_SCENE:
                sceneManager.LoadScene("Physics Test");
                showMainMenu = false;
                physicsScene->SetPhysicsEnabled(physicsEnabled);
                physicsScene->SetTimeScale(physicsTimeScale);
                std::cout << "Entered Physics Test Scene" << std::endl;
                break;

            case AppState::PARTICLE_SCENE:  // Handle particle scene entry
                sceneManager.LoadScene("Particle Physics");
                showMainMenu = false;
                particleScene->SetPhysicsEnabled(particlePhysicsEnabled);
                particleScene->SetTimeScale(particleTimeScale);
                particleScene->SetParticleRadius(particleRadius);
                std::cout << "Entered Particle Physics Scene" << std::endl;
                break;

            case AppState::EXITING:
                std::cout << "Exiting application..." << std::endl;
                break;
        }

        stateTransitionPending = false;
    }

    void Update(float deltaTime) {
        // Process scene transitions
        sceneManager.ProcessSceneTransition();

        // Update current state
        switch (currentState) {
            case AppState::MAIN_MENU:
                // Menu doesn't need updates
                break;

            case AppState::DEMO_SCENE:
                demoScene->Update(deltaTime);
                break;

            case AppState::PHYSICS_SCENE:
                physicsScene->Update(deltaTime);
                break;

            case AppState::PARTICLE_SCENE:  // Update particle scene
                particleScene->Update(deltaTime);
                break;

            case AppState::EXITING:
                break;
        }
    }

    void Render() {
        renderer.BeginFrame();

        // Render current scene
        if (currentState == AppState::PARTICLE_SCENE) {
            RenderParticleScene();  // Special 2D rendering for particle scene
        } else if (currentState != AppState::MAIN_MENU) {
            RenderCurrentScene();
        }

        // Render UI
        RenderUI();

        renderer.EndFrame();
    }

    void RenderCurrentScene() {
        auto currentScene = sceneManager.GetCurrentScene();
        if (!currentScene) return;

        // Render all entities in the current scene (3D rendering)
        for (const auto& entity : currentScene->GetEntities()) {
            if (!entity || !entity->IsActive()) continue;

            auto transform = entity->GetComponent<Engine::Logic::TransformComponent>();
            auto renderComp = entity->GetComponent<Engine::Logic::RenderComponent>();

            if (transform && renderComp && renderComp->IsVisible()) {
                if (renderComp->GetPrimitiveType() == Engine::Logic::PrimitiveType::CUBE) {
                    // Use the complete transformation matrix instead of just position and scale
                    renderer.RenderCube(transform->GetTransformMatrix());
                }
            }
        }
    }

    void RenderParticleScene() {
        // Switch to 2D rendering mode
        renderer.BeginRender2D();

        auto currentScene = sceneManager.GetCurrentScene();
        if (currentScene) {
            // Use batch rendering for better performance with many particles
            renderer.BeginBatch();

            for (const auto& entity : currentScene->GetEntities()) {
                if (!entity || !entity->IsActive()) continue;

                auto transform = entity->GetComponent<Engine::Logic::TransformComponent>();
                auto renderComp = entity->GetComponent<Engine::Logic::RenderComponent>();

                if (transform && renderComp && renderComp->IsVisible()) {
                    glm::vec3 pos3D = transform->GetPosition();
                    glm::vec2 pos2D = glm::vec2(pos3D.x, pos3D.y);

                    if (renderComp->GetPrimitiveType() == Engine::Logic::PrimitiveType::CIRCLE) {
                        // Add circles to batch for particles
                        auto collision = entity->GetComponent<Engine::Logic::CollisionComponent>();
                        float radius = collision ? collision->GetCircle().radius : 5.0f;
                        renderer.AddCircleToBatch(pos2D, radius, renderComp->GetColor());
                    } else if (renderComp->GetPrimitiveType() == Engine::Logic::PrimitiveType::CUBE) {
                        // Render walls as rectangles
                        glm::vec3 scale = transform->GetScale();
                        glm::vec2 size = glm::vec2(scale.x, scale.y);
                        renderer.RenderRect2D(pos2D, size, renderComp->GetColor());
                    }
                }
            }

            renderer.RenderBatch();
            renderer.EndBatch();
        }

        renderer.EndRender2D();
    }

    void RenderUI() {
        if (showMainMenu) {
            RenderMainMenu();
        } else {
            RenderSceneUI();
        }

        if (showDebugInfo) {
            RenderDebugInfo();
        }
    }

    void RenderMainMenu() {
        // Center the main menu
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 windowSize(400, 350);  // Increased height for new scene
        ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);

        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

        if (ImGui::Begin("BasicEngine Main Menu", nullptr, flags)) {
            // Title
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font but bigger
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("BasicEngine").x) * 0.5f);
            ImGui::Text("BasicEngine");
            ImGui::PopFont();

            ImGui::Separator();
            ImGui::Spacing();

            // Scene selection buttons
            ImGui::Text("Select a Scene:");
            ImGui::Spacing();

            if (ImGui::Button("Demo Scene", ImVec2(350, 40))) {
                RequestStateChange(AppState::DEMO_SCENE);
            }
            ImGui::Text(" • Moving and rotating cubes");
            ImGui::Text(" • Basic animation showcase");

            ImGui::Spacing();

            if (ImGui::Button("Physics Test", ImVec2(350, 40))) {
                RequestStateChange(AppState::PHYSICS_SCENE);
            }
            ImGui::Text(" • Falling and bouncing objects");
            ImGui::Text(" • Simple physics simulation");

            ImGui::Spacing();

            if (ImGui::Button("Particle Physics", ImVec2(350, 40))) {  // New particle scene button
                RequestStateChange(AppState::PARTICLE_SCENE);
            }
            ImGui::Text(" • 2D particle simulation");
            ImGui::Text(" • Collision detection and response");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Options
            ImGui::Checkbox("Show Debug Info", &showDebugInfo);

            ImGui::Spacing();

            if (ImGui::Button("Exit", ImVec2(350, 30))) {
                RequestStateChange(AppState::EXITING);
            }
        }
        ImGui::End();
    }

    void RenderSceneUI() {
        // Back to menu button (always visible)
        if (ImGui::Begin("Scene Controls")) {
            if (ImGui::Button("Back to Main Menu")) {
                RequestStateChange(AppState::MAIN_MENU);
            }

            ImGui::Separator();

            // Scene-specific controls
            switch (currentState) {
                case AppState::DEMO_SCENE:
                    RenderDemoSceneControls();
                    break;

                case AppState::PHYSICS_SCENE:
                    RenderPhysicsSceneControls();
                    break;

                case AppState::PARTICLE_SCENE:  // New particle scene controls
                    RenderParticleSceneControls();
                    break;

                default:
                    break;
            }

            ImGui::Separator();
            ImGui::Checkbox("Show Debug Info", &showDebugInfo);
        }
        ImGui::End();
    }

    void RenderDemoSceneControls() {
        ImGui::Text("Demo Scene Controls");

        // Animation speed control
        float speed = demoScene->GetAnimationSpeed();
        if (ImGui::SliderFloat("Animation Speed", &speed, 0.1f, 5.0f)) {
            demoScene->SetAnimationSpeed(speed);
        }

        // Entity controls
        if (ImGui::TreeNode("Entity Controls")) {
            auto scene = demoScene->GetScene();
            for (const auto& entity : scene->GetEntities()) {
                if (ImGui::TreeNode(entity->GetName().c_str())) {
                    auto transform = entity->GetComponent<Engine::Logic::TransformComponent>();
                    auto renderComp = entity->GetComponent<Engine::Logic::RenderComponent>();

                    if (transform) {
                        glm::vec3 pos = transform->GetPosition();
                        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                            transform->SetPosition(pos);
                        }

                        glm::vec3 scale = transform->GetScale();
                        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 5.0f)) {
                            transform->SetScale(scale);
                        }
                    }

                    if (renderComp) {
                        bool visible = renderComp->IsVisible();
                        if (ImGui::Checkbox("Visible", &visible)) {
                            renderComp->SetVisible(visible);
                        }
                    }

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }

    void RenderPhysicsSceneControls() {
        ImGui::Text("Physics Test Controls");

        // Physics controls
        if (ImGui::Checkbox("Enable Physics", &physicsEnabled)) {
            physicsScene->SetPhysicsEnabled(physicsEnabled);
        }

        if (ImGui::SliderFloat("Time Scale", &physicsTimeScale, 0.0f, 3.0f)) {
            physicsScene->SetTimeScale(physicsTimeScale);
        }

        if (ImGui::Button("Reset Scene")) {
            physicsScene->Reset();
        }

        ImGui::SameLine();
        if (ImGui::Button("Spawn Random Cube")) {
            physicsScene->SpawnRandomCube();
        }

        // Physics entity info
        if (ImGui::TreeNode("Physics Objects")) {
            auto scene = physicsScene->GetScene();
            for (const auto& entity : scene->GetEntities()) {
                if (ImGui::TreeNode(entity->GetName().c_str())) {
                    auto transform = entity->GetComponent<Engine::Logic::TransformComponent>();
                    auto physics = entity->GetComponent<Engine::Logic::SimplePhysicsComponent>();

                    if (transform) {
                        glm::vec3 pos = transform->GetPosition();
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
                    }

                    if (physics) {
                        glm::vec3 vel = physics->GetVelocity();
                        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);
                        ImGui::Text("Mass: %.2f", physics->GetMass());

                        if (ImGui::Button("Add Upward Force")) {
                            physics->ApplyImpulse(glm::vec3(0.0f, 5.0f, 0.0f));
                        }
                    }

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }

    void RenderParticleSceneControls() {  // New particle scene controls
        ImGui::Text("2D Particle Physics Controls");

        // Physics controls
        if (ImGui::Checkbox("Enable Physics", &particlePhysicsEnabled)) {
            particleScene->SetPhysicsEnabled(particlePhysicsEnabled);
        }

        if (ImGui::SliderFloat("Time Scale", &particleTimeScale, 0.0f, 3.0f)) {
            particleScene->SetTimeScale(particleTimeScale);
        }

        ImGui::Separator();

        // Particle spawning controls
        ImGui::Text("Particle Spawning");

        if (ImGui::SliderFloat("Particle Size", &particleRadius,
                              particleScene->GetMinParticleRadius(),
                              particleScene->GetMaxParticleRadius())) {
            particleScene->SetParticleRadius(particleRadius);
        }

        if (ImGui::Button("Spawn Particle", ImVec2(120, 30))) {
            particleScene->SpawnParticle();
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear All", ImVec2(80, 30))) {
            particleScene->ClearAllParticles();
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset Scene", ImVec2(90, 30))) {
            particleScene->Reset();
        }

        ImGui::Separator();

        // Particle scene info
        ImGui::Text("Particle Count: %zu", particleScene->GetParticleCount());
        ImGui::Text("Cup Dimensions: %.0fx%.0f pixels",
                   particleScene->GetCupWidth(), particleScene->GetCupHeight());

        // Collision system info
        if (ImGui::TreeNode("Collision System")) {
            auto collisionSystem = particleScene->GetCollisionSystem();
            if (collisionSystem) {
                ImGui::Text("%s", collisionSystem->GetDebugInfo().c_str());
            }
            ImGui::TreePop();
        }
    }

    void RenderDebugInfo() {
        if (ImGui::Begin("Debug Information")) {
            ImGui::Text("Application State: %s", GetStateString(currentState));
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

            ImGui::Separator();

            // Scene manager info
            ImGui::Text("%s", sceneManager.GetDebugInfo().c_str());

            // Current scene info
            auto currentScene = sceneManager.GetCurrentScene();
            if (currentScene) {
                ImGui::Separator();
                ImGui::Text("Current Scene Debug:");
                ImGui::Text("%s", currentScene->GetDebugInfo().c_str());
            }

            // Particle scene specific debug info
            if (currentState == AppState::PARTICLE_SCENE) {
                ImGui::Separator();
                ImGui::Text("Particle Scene Debug:");
                auto collisionSystem = particleScene->GetCollisionSystem();
                if (collisionSystem) {
                    ImGui::Text("%s", collisionSystem->GetDebugInfo().c_str());
                }
            }
        }
        ImGui::End();
    }

    void RequestStateChange(AppState newState) {
        nextState = newState;
        stateTransitionPending = true;
    }

    const char* GetStateString(AppState state) {
        switch (state) {
            case AppState::MAIN_MENU: return "Main Menu";
            case AppState::DEMO_SCENE: return "Demo Scene";
            case AppState::PHYSICS_SCENE: return "Physics Scene";
            case AppState::PARTICLE_SCENE: return "Particle Scene";  // New state string
            case AppState::EXITING: return "Exiting";
            default: return "Unknown";
        }
    }
};

int main() {
    BasicEngineApp app;

    if (!app.Initialize()) {
        return -1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}
