#include "../../../renderer/include/ImGuiManager.h"
#include "../../Common/include/RendererWrapper.h"
#include "../../Logic/include/DemoScene.h"
#include "../../Logic/include/ParticleScene.h"
#include "../../Logic/include/PhysicsTestScene.h"
#include "../../Logic/include/SceneManager.h"
#include <glm/glm.hpp>
#include <iostream>

enum class AppState { MAIN_MENU, DEMO_SCENE, PHYSICS_SCENE, EXITING };

class BasicEngineApp {
private:
  Engine::Common::OpenGLRendererWrapper renderer;
  Engine::Logic::SceneManager sceneManager;

  std::unique_ptr<Engine::Logic::DemoScene> demoScene;
  std::unique_ptr<Engine::Logic::PhysicsTestScene> physicsScene;

  AppState currentState = AppState::MAIN_MENU;
  AppState nextState = AppState::MAIN_MENU;
  bool stateTransitionPending = false;

  bool showMainMenu = true;
  bool showDebugInfo = false;

  bool physicsEnabled = true;
  float physicsTimeScale = 1.0f;
  // float particleRadius = 8.0f;
  // bool particlePhysicsEnabled = true;
  // float particleTimeScale = 1.0f;

public:
  bool Initialize() {
    if (!renderer.Initialize(1920, 1080, "BasicEngine - Scene Manager Demo")) {
      std::cerr << "Failed to initialize renderer" << std::endl;
      return false;
    }

    // Create scene instances
    demoScene = std::make_unique<Engine::Logic::DemoScene>();
    physicsScene = std::make_unique<Engine::Logic::PhysicsTestScene>();

    // Register scenes with the scene manager
    sceneManager.RegisterScene("Demo", demoScene->GetScene());
    sceneManager.RegisterScene("Physics Test", physicsScene->GetScene());

    std::cout << "BasicEngine initialized successfully!" << std::endl;
    sceneManager.PrintSceneList();

    return true;
  }

  void Run() {
    float lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(renderer.GetWindow()) &&
           currentState != AppState::EXITING) {
      // Calculate delta time
      float currentTime = glfwGetTime();
      float deltaTime = currentTime - lastFrameTime;
      lastFrameTime = currentTime;

      ProcessStateTransition();
      Update(deltaTime);
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
    if (!stateTransitionPending)
      return;

    switch (currentState) {
    case AppState::DEMO_SCENE:
    case AppState::PHYSICS_SCENE:
    default:
      break;
    }

    currentState = nextState;

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
    sceneManager.ProcessSceneTransition();

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

  void Render() {
    renderer.BeginFrame();
    if (currentState != AppState::MAIN_MENU) {
      RenderCurrentScene();
    }
    RenderUI();
    renderer.EndFrame();
  }

  void RenderCurrentScene() {
    auto currentScene = sceneManager.GetCurrentScene();
    if (!currentScene)
      return;

    for (const auto &entity : currentScene->GetEntities()) {
      if (!entity || !entity->IsActive())
        continue;

      auto transform =
          entity->GetComponent<Engine::Logic::TransformComponent>();
      auto renderComp = entity->GetComponent<Engine::Logic::RenderComponent>();

      if (transform && renderComp && renderComp->IsVisible()) {
        if (renderComp->GetPrimitiveType() ==
            Engine::Logic::PrimitiveType::CUBE) {
          renderer.RenderCube(transform->GetTransformMatrix());
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
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 windowSize(400, 350); // Increased height for new scene
    ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f,
                     (io.DisplaySize.y - windowSize.y) * 0.5f);

    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(windowPos);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("BasicEngine Main Menu", nullptr, flags)) {
      // Title
      ImGui::PushFont(
          ImGui::GetIO().Fonts->Fonts[0]); // Use default font but bigger
      ImGui::SetCursorPosX(
          (ImGui::GetWindowSize().x - ImGui::CalcTextSize("BasicEngine").x) *
          0.5f);
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
      for (const auto &entity : scene->GetEntities()) {
        if (ImGui::TreeNode(entity->GetName().c_str())) {
          auto transform =
              entity->GetComponent<Engine::Logic::TransformComponent>();
          auto renderComp =
              entity->GetComponent<Engine::Logic::RenderComponent>();

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
      for (const auto &entity : scene->GetEntities()) {
        if (ImGui::TreeNode(entity->GetName().c_str())) {
          auto transform =
              entity->GetComponent<Engine::Logic::TransformComponent>();
          auto physics =
              entity->GetComponent<Engine::Logic::SimplePhysicsComponent>();

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

      ImGui::Text("%s", sceneManager.GetDebugInfo().c_str());

      auto currentScene = sceneManager.GetCurrentScene();
      if (currentScene) {
        ImGui::Separator();
        ImGui::Text("Current Scene Debug:");
        ImGui::Text("%s", currentScene->GetDebugInfo().c_str());
      }

      ImGui::End();
    }
  }

  void RequestStateChange(AppState newState) {
    nextState = newState;
    stateTransitionPending = true;
  }

  const char *GetStateString(AppState state) {
    switch (state) {
    case AppState::MAIN_MENU:
      return "Main Menu";
    case AppState::DEMO_SCENE:
      return "Demo Scene";
    case AppState::PHYSICS_SCENE:
      return "Physics Scene";
    case AppState::EXITING:
      return "Exiting";
    default:
      return "Unknown";
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
