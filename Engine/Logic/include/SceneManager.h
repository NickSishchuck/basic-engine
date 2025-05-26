#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine {
namespace Logic {

class SceneManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;
    std::shared_ptr<Scene> currentScene;
    std::string currentSceneName;
    std::shared_ptr<Scene> nextScene; // For scene transitions
    bool sceneTransitionPending = false;

public:
    SceneManager() = default;
    ~SceneManager() = default;

    // Scene management
    void RegisterScene(const std::string& name, std::shared_ptr<Scene> scene);
    bool LoadScene(const std::string& name);
    void UnloadCurrentScene();

    // Scene access
    std::shared_ptr<Scene> GetCurrentScene() const { return currentScene; }
    const std::string& GetCurrentSceneName() const { return currentSceneName; }
    std::shared_ptr<Scene> GetScene(const std::string& name);

    // Available scenes
    std::vector<std::string> GetAvailableScenes() const;
    bool HasScene(const std::string& name) const;

    // Lifecycle
    void Update(float deltaTime);
    void ProcessSceneTransition(); // Call this in your main loop

    // Debug
    std::string GetDebugInfo() const;
    void PrintSceneList() const;
};

} // namespace Logic
} // namespace Engine

#endif
