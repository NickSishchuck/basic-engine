#include "../../Common/include/RendererWrapper.h"
#include "../../Logic/include/SceneManager.h"
#include "../../../renderer/include/ImGuiManager.h"
#include "../../Logic/include/DemoScene.h"
#include "../../Logic/include/PhysicsTestScene.h"
#include <iostream>
#include <glm/glm.hpp>

// Application states
enum class AppState {
    MAIN_MENU,
    DEMO_SCENE,
    PHYSICS_SCENE,
    EXITING
};

class PlaneEngineApp {
private:

    // Docking and viewport
    bool showViewport = true;
    bool showSceneHierarchy = true;
    bool showInspector = true;
    bool showConsole = true;
    ImVec2 lastViewportSize = ImVec2(800, 600);

    // Selected entity for inspector
    std::shared_ptr<Engine::Logic::Entity> selectedEntity = nullptr;
    Engine::Common::OpenGLRendererWrapper renderer;
    Engine::Logic::SceneManager sceneManager;

    // Scene instances
    std::unique_ptr<Engine::Logic::DemoScene> demoScene;
    std::unique_ptr<Engine::Logic::PhysicsTestScene> physicsScene;

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

public:
    bool Initialize() {
        if (!renderer.Initialize(1920, 1080, "PlaneEngine - Scene Manager Demo")) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        // Create scene instances
        demoScene = std::make_unique<Engine::Logic::DemoScene>();
        physicsScene = std::make_unique<Engine::Logic::PhysicsTestScene>();

        // Register scenes with the scene manager
        sceneManager.RegisterScene("Demo", demoScene->GetScene());
        sceneManager.RegisterScene("Physics Test", physicsScene->GetScene());

        std::cout << "PlaneEngine initialized successfully!" << std::endl;
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
        renderer.Shutdown();
    }

private:
    void ProcessStateTransition() {
        if (!stateTransitionPending) return;

        // Handle state exit
        switch (currentState) {
            case AppState::DEMO_SCENE:
            case AppState::PHYSICS_SCENE:
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

            case AppState::EXITING:
                break;
        }
    }

    void PlaneEngineApp::Render() {
        // First, render the 3D scene to the framebuffer
        renderer.BeginViewportRender();
        RenderCurrentScene();
        renderer.EndViewportRender();

        // Now render the main window with ImGui
        renderer.BeginFrame();

        // Create the dockspace
        CreateDockSpace();

        // Render all UI panels
        if (showViewport) RenderViewport();
        if (showSceneHierarchy) RenderSceneHierarchy();
        if (showInspector) RenderInspector();
        if (showConsole) RenderConsole();

        // Render any floating windows (main menu, etc.)
        if (showMainMenu) {
            RenderMainMenu();
        }

        renderer.EndFrame();
    }

    void PlaneEngineApp::CreateDockSpace() {
        static bool dockspaceOpen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar(3);

        // Submit the DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        // Menu Bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    RequestStateChange(AppState::EXITING);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Viewport", nullptr, &showViewport);
                ImGui::MenuItem("Scene Hierarchy", nullptr, &showSceneHierarchy);
                ImGui::MenuItem("Inspector", nullptr, &showInspector);
                ImGui::MenuItem("Console", nullptr, &showConsole);
                ImGui::Separator();
                ImGui::MenuItem("Main Menu", nullptr, &showMainMenu);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    // Add the viewport rendering method:
    void PlaneEngineApp::RenderViewport() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin("Viewport", &showViewport)) {
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            // Check if viewport size changed
            if (viewportSize.x != lastViewportSize.x || viewportSize.y != lastViewportSize.y) {
                if (viewportSize.x > 0 && viewportSize.y > 0) {
                    renderer.ResizeViewport((int)viewportSize.x, (int)viewportSize.y);
                    lastViewportSize = viewportSize;
                }
            }

            // Display the rendered texture
            ImGui::Image(
                (void*)(intptr_t)renderer.GetViewportTexture(),
                viewportSize,
                ImVec2(0, 1),  // UV0 (flip Y)
                ImVec2(1, 0)   // UV1 (flip Y)
            );

            // TODO: Handle viewport-specific input when focused
            // if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
            //     // Handle camera input here
            // }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // Add the scene hierarchy panel:
    void PlaneEngineApp::RenderSceneHierarchy() {
        if (ImGui::Begin("Scene Hierarchy", &showSceneHierarchy)) {
            auto currentScene = sceneManager.GetCurrentScene();
            if (currentScene) {
                ImGui::Text("Scene: %s", currentScene->GetName().c_str());
                ImGui::Separator();

                for (const auto& entity : currentScene->GetEntities()) {
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                             ImGuiTreeNodeFlags_OpenOnDoubleClick;

                    if (selectedEntity == entity) {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }

                    bool nodeOpen = ImGui::TreeNodeEx(
                        (void*)(intptr_t)entity->GetID(),
                        flags,
                        "%s", entity->GetName().c_str()
                    );

                    if (ImGui::IsItemClicked()) {
                        selectedEntity = entity;
                    }

                    if (nodeOpen) {
                        for (const auto& component : entity->GetAllComponents()) {
                            ImGui::BulletText("%s", component->GetTypeName().c_str());
                        }
                        ImGui::TreePop();
                    }
                }
            } else {
                ImGui::Text("No scene loaded");
            }
        }
        ImGui::End();
    }

    // Add the inspector panel:
    void PlaneEngineApp::RenderInspector() {
        if (ImGui::Begin("Inspector", &showInspector)) {
            if (selectedEntity) {
                ImGui::Text("Entity: %s", selectedEntity->GetName().c_str());
                ImGui::Text("ID: %d", selectedEntity->GetID());
                ImGui::Separator();

                // Transform component
                auto transform = selectedEntity->GetComponent<Engine::Logic::TransformComponent>();
                if (transform && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    glm::vec3 pos = transform->GetPosition();
                    if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                        transform->SetPosition(pos);
                    }

                    glm::vec3 rot = transform->GetRotation();
                    if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f)) {
                        transform->SetRotation(rot);
                    }

                    glm::vec3 scale = transform->GetScale();
                    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.1f, 5.0f)) {
                        transform->SetScale(scale);
                    }
                }

                // Render component
                auto render = selectedEntity->GetComponent<Engine::Logic::RenderComponent>();
                if (render && ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) {
                    bool visible = render->IsVisible();
                    if (ImGui::Checkbox("Visible", &visible)) {
                        render->SetVisible(visible);
                    }

                    // Color picker could go here if you add color support
                }

                // Physics component (if in physics scene)
                auto physics = selectedEntity->GetComponent<Engine::Logic::SimplePhysicsComponent>();
                if (physics && ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                    glm::vec3 velocity = physics->GetVelocity();
                    ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity.x, velocity.y, velocity.z);
                    ImGui::Text("Mass: %.2f", physics->GetMass());

                    if (ImGui::Button("Apply Upward Force")) {
                        physics->ApplyImpulse(glm::vec3(0.0f, 5.0f, 0.0f));
                    }
                }
            } else {
                ImGui::Text("No entity selected");
            }
        }
        ImGui::End();
    }

    // Add the console panel:
    void PlaneEngineApp::RenderConsole() {
        if (ImGui::Begin("Console", &showConsole)) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Separator();

            if (ImGui::Button("Clear")) {
                // TODO: Clear console log
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy")) {
                // TODO: Copy console contents
            }

            ImGui::Separator();

            // TODO: Add actual console/log output here
            ImGui::TextWrapped("Console output will appear here...");

            // You could integrate with your Logger class here
        }
        ImGui::End();
    }


    void RenderCurrentScene() {
        auto currentScene = sceneManager.GetCurrentScene();
        if (!currentScene) return;

        // Render all entities in the current scene
        for (const auto& entity : currentScene->GetEntities()) {
            if (!entity || !entity->IsActive()) continue;

            auto transform = entity->GetComponent<Engine::Logic::TransformComponent>();
            auto renderComp = entity->GetComponent<Engine::Logic::RenderComponent>();

            if (transform && renderComp && renderComp->IsVisible()) {
                if (renderComp->GetPrimitiveType() == Engine::Logic::PrimitiveType::CUBE) {
                    renderer.RenderCube(transform->GetPosition(), transform->GetScale());
                }
            }
        }
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
        ImVec2 windowSize(400, 300);
        ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);

        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

        if (ImGui::Begin("PlaneEngine Main Menu", nullptr, flags)) {
            // Title
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font but bigger
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("PlaneEngine").x) * 0.5f);
            ImGui::Text("PlaneEngine");
            ImGui::PopFont();

            ImGui::Separator();
            ImGui::Spacing();

            // Scene selection buttons
            ImGui::Text("Select a Scene:");
            ImGui::Spacing();

            if (ImGui::Button("Demo Scene", ImVec2(350, 40))) {
                RequestStateChange(AppState::DEMO_SCENE);
            }
            ImGui::Text(" ? Moving and rotating cubes");
            ImGui::Text(" ? Basic animation showcase");

            ImGui::Spacing();

            if (ImGui::Button("Physics Test", ImVec2(350, 40))) {
                RequestStateChange(AppState::PHYSICS_SCENE);
            }
            ImGui::Text(" ? Falling and bouncing objects");
            ImGui::Text(" ? Simple physics simulation");

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
            case AppState::EXITING: return "Exiting";
            default: return "Unknown";
        }
    }
};

int main() {
    PlaneEngineApp app;

    if (!app.Initialize()) {
        return -1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}
