/**
 * End Dimension Viewer - Main Application
 * 
 * This demonstrates how to integrate the EndRenderer with BasicEngine's
 * existing infrastructure. The key differences from the standard engine:
 * 
 * 1. No ECS/Scene system - terrain is procedurally generated
 * 2. Single renderer doing ray marching instead of geometry
 * 3. Double-precision camera for cosmic scale navigation
 * 
 * Build:
 *   Add EndViewer module to CMakeLists.txt
 *   Link against Common (for Shader, VAO, VBO, ImGui)
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// BasicEngine components we reuse
#include "../../../renderer/include/ImGuiManager.h"
#include "../../../renderer/include/Logger.h"

// End viewer components
#include "../../EndViewer/include/EndRenderer.h"

// Error callback for GLFW
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Window resize callback
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize Logger
    Logger* logger = Logger::getInstance();
    logger->enableColors(true);
    
    if (!logger->init()) {
        std::cerr << "Failed to initialize logger!" << std::endl;
        return -1;
    }
    
    LOG_INFO("=== End Dimension Viewer ===");
    LOG_INFO("Initializing...");
    
    // Initialize GLFW
    if (!glfwInit()) {
        LOG_FATAL("Failed to initialize GLFW");
        return -1;
    }
    glfwSetErrorCallback(glfwErrorCallback);
    
    // OpenGL context settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    int windowWidth = 1920;
    int windowHeight = 1080;
    GLFWwindow* window = glfwCreateWindow(
        windowWidth, windowHeight, 
        "End Dimension Viewer - Endscope v0.1", 
        nullptr, nullptr
    );
    
    if (!window) {
        LOG_FATAL("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSwapInterval(1);  // VSync
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        LOG_FATAL("Failed to initialize GLEW");
        glfwTerminate();
        return -1;
    }
    
    // Clear any GLEW errors
    while (glGetError() != GL_NO_ERROR);
    
    LOG_INFO("OpenGL Version: " + std::string((const char*)glGetString(GL_VERSION)));
    LOG_INFO("Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
    
    // Initialize ImGui
    ImGuiManager imguiManager(window);
    imguiManager.Initialize();
    LOG_INFO("ImGui initialized");
    
    // Initialize End renderer
    EndViewer::EndRenderer endRenderer;
    if (!endRenderer.initialize(window, &imguiManager)) {
        LOG_FATAL("Failed to initialize End renderer");
        imguiManager.Shutdown();
        glfwTerminate();
        return -1;
    }
    LOG_INFO("End renderer initialized");
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Main loop
    LOG_INFO("Entering main loop...");
    LOG_INFO("Controls: WASD to move, Space/Shift for up/down, Right-click + drag to look");
    
    float lastFrameTime = static_cast<float>(glfwGetTime());
    bool showUI = true;
    bool wasTabPressed = false;
    
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        // Poll events
        glfwPollEvents();
        
        // Handle UI toggle
        bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
        if (tabPressed && !wasTabPressed) {
            showUI = !showUI;
            endRenderer.getSettings().showDebugUI = showUI;
        }
        wasTabPressed = tabPressed;
        
        // Handle escape to quit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        
        // Begin ImGui frame
        imguiManager.BeginFrame();
        
        // Render the End dimension
        endRenderer.renderFrame(deltaTime);
        
        // End ImGui frame
        imguiManager.EndFrame();
        imguiManager.Render();
        
        // Swap buffers
        glfwSwapBuffers(window);
    }
    
    LOG_INFO("Shutting down...");
    
    // Cleanup
    endRenderer.shutdown();
    imguiManager.Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    LOG_INFO("Application terminated normally");
    return 0;
}
