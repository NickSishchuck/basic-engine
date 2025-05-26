#include "../include/SceneManager.h"
#include <iostream>
#include <algorithm>

namespace Engine {
namespace Logic {

void SceneManager::RegisterScene(const std::string& name, std::shared_ptr<Scene> scene) {
    if (!scene) {
        std::cerr << "Warning: Attempting to register null scene: " << name << std::endl;
        return;
    }

    scenes[name] = scene;
    scene->SetName(name);
    std::cout << "Registered scene: " << name << std::endl;
}

bool SceneManager::LoadScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it == scenes.end()) {
        std::cerr << "Error: Scene not found: " << name << std::endl;
        return false;
    }

    // Mark for transition (don't switch immediately to avoid mid-frame issues)
    nextScene = it->second;
    sceneTransitionPending = true;

    return true;
}

void SceneManager::ProcessSceneTransition() {
    if (!sceneTransitionPending) return;

    // Unload current scene
    if (currentScene) {
        std::cout << "Unloading scene: " << currentSceneName << std::endl;
        currentScene->SetActive(false);
    }

    // Load new scene
    currentScene = nextScene;
    currentSceneName = "";

    // Find the scene name
    for (const auto& pair : scenes) {
        if (pair.second == currentScene) {
            currentSceneName = pair.first;
            break;
        }
    }

    if (currentScene) {
        std::cout << "Loading scene: " << currentSceneName << std::endl;
        currentScene->SetActive(true);
    }

    // Clear transition state
    nextScene = nullptr;
    sceneTransitionPending = false;
}

void SceneManager::UnloadCurrentScene() {
    if (currentScene) {
        currentScene->SetActive(false);
        currentScene = nullptr;
        currentSceneName = "";
    }
}

std::shared_ptr<Scene> SceneManager::GetScene(const std::string& name) {
    auto it = scenes.find(name);
    return (it != scenes.end()) ? it->second : nullptr;
}

std::vector<std::string> SceneManager::GetAvailableScenes() const {
    std::vector<std::string> sceneNames;
    for (const auto& pair : scenes) {
        sceneNames.push_back(pair.first);
    }
    std::sort(sceneNames.begin(), sceneNames.end());
    return sceneNames;
}

bool SceneManager::HasScene(const std::string& name) const {
    return scenes.find(name) != scenes.end();
}

void SceneManager::Update(float deltaTime) {
    if (currentScene && currentScene->IsActive()) {
        currentScene->Update(deltaTime);
    }
}

std::string SceneManager::GetDebugInfo() const {
    std::string info = "=== Scene Manager ===\n";
    info += "Current Scene: " + (currentScene ? currentSceneName : "None") + "\n";
    info += "Available Scenes (" + std::to_string(scenes.size()) + "):\n";

    for (const auto& pair : scenes) {
        info += "  - " + pair.first;
        if (pair.second == currentScene) {
            info += " (ACTIVE)";
        }
        info += "\n";
    }

    if (sceneTransitionPending) {
        info += "Pending transition to new scene...\n";
    }

    return info;
}

void SceneManager::PrintSceneList() const {
    std::cout << GetDebugInfo() << std::endl;
}

} // namespace Logic
} // namespace Engine
