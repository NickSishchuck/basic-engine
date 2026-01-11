#ifndef END_RENDERER_H
#define END_RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <iostream>

// Include the existing renderer components we'll reuse
#include "../../../renderer/include/shaderClass.h"
#include "../../../renderer/include/VAO.h"
#include "../../../renderer/include/VBO.h"
#include "../../../renderer/include/ImGuiManager.h"

#include "EndCamera.h"
#include "EndDensity.h"

namespace EndViewer {

/**
 * End Dimension Renderer
 * 
 * This renderer uses GPU ray marching to render Minecraft's End dimension
 * terrain at arbitrary scales, from block-level detail to cosmic views
 * of the ring structure.
 * 
 * Integrates with BasicEngine's existing infrastructure:
 * - Uses the Shader class for GLSL loading
 * - Uses VAO/VBO for the fullscreen quad
 * - Uses ImGuiManager for debug UI
 */
class EndRenderer {
public:
    // Render settings
    struct Settings {
        // Ray marching parameters
        int maxSteps = 256;
        float maxDistance = 50000.0f;
        float stepMultiplier = 1.0f;
        
        // Quality settings (adjusted by LOD)
        int baseOctaves = 4;
        
        // Colors
        glm::vec3 endStoneColor = glm::vec3(0.85f, 0.85f, 0.65f);  // Pale yellow
        glm::vec3 skyColor = glm::vec3(0.0f, 0.0f, 0.05f);         // Near black
        glm::vec3 fogColor = glm::vec3(0.1f, 0.05f, 0.15f);        // Purple tint
        float fogDensity = 1.0f;
        
        // Debug
        bool showDebugUI = true;
        bool wireframeMode = false;
    };
    
private:
    // Window reference
    GLFWwindow* window;
    int width, height;
    
    // Shader
    std::unique_ptr<Shader> rayMarchShader;
    
    // Fullscreen quad geometry
    std::unique_ptr<VAO> quadVAO;
    std::unique_ptr<VBO> quadVBO;
    
    // Camera
    std::unique_ptr<EndCamera> camera;
    
    // CPU density function (for testing/verification)
    std::unique_ptr<EndDensity> cpuDensity;
    
    // ImGui manager (owned externally, just a reference)
    ImGuiManager* imguiManager;
    
    // Settings
    Settings settings;
    
    // Performance tracking
    float lastFrameTime = 0.0f;
    float frameTimeAccum = 0.0f;
    int frameCount = 0;
    float averageFPS = 0.0f;
    
public:
    /**
     * Initialize the End renderer
     * @param existingWindow GLFW window from BasicEngine
     * @param existingImGui ImGuiManager from BasicEngine
     */
    bool initialize(GLFWwindow* existingWindow, ImGuiManager* existingImGui) {
        window = existingWindow;
        imguiManager = existingImGui;
        
        // Get window size
        glfwGetFramebufferSize(window, &width, &height);
        
        std::cout << "EndRenderer: Initializing..." << std::endl;
        
        // Create camera
        camera = std::make_unique<EndCamera>(width, height, glm::dvec3(0.0, 100.0, 200.0));
        std::cout << "EndRenderer: Camera created" << std::endl;
        
        // Create CPU density function (seed 0 for testing)
        cpuDensity = std::make_unique<EndDensity>(0);
        std::cout << "EndRenderer: CPU density function created" << std::endl;
        
        // Create fullscreen quad
        if (!createFullscreenQuad()) {
            std::cerr << "EndRenderer: Failed to create fullscreen quad" << std::endl;
            return false;
        }
        std::cout << "EndRenderer: Fullscreen quad created" << std::endl;
        
        // Load ray marching shader
        if (!loadShaders()) {
            std::cerr << "EndRenderer: Failed to load shaders" << std::endl;
            return false;
        }
        std::cout << "EndRenderer: Shaders loaded" << std::endl;
        
        // Verify density function at known coordinates
        verifyDensityFunction();
        
        std::cout << "EndRenderer: Initialization complete!" << std::endl;
        return true;
    }
    
    /**
     * Update and render frame
     */
    void renderFrame(float deltaTime) {
        // Update performance metrics
        updatePerformanceMetrics(deltaTime);
        
        // Handle input
        camera->handleInput(window, deltaTime);
        
        // Update window size
        int newWidth, newHeight;
        glfwGetFramebufferSize(window, &newWidth, &newHeight);
        if (newWidth != width || newHeight != height) {
            width = newWidth;
            height = newHeight;
            camera->setViewportSize(width, height);
        }
        
        // Calculate LOD-adjusted settings
        float lod = camera->getLODFactor();
        int octaves = std::max(1, settings.baseOctaves - static_cast<int>(lod));
        float stepMult = settings.stepMultiplier * std::pow(2.0f, lod);
        
        // Clear screen
        glClearColor(settings.skyColor.r, settings.skyColor.g, settings.skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render terrain
        rayMarchShader->Activate();
        setShaderUniforms(octaves, stepMult);
        
        quadVAO->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        quadVAO->Unbind();
        
        // Render debug UI
        if (settings.showDebugUI) {
            renderDebugUI();
        }
    }
    
    /**
     * Cleanup resources
     */
    void shutdown() {
        if (quadVAO) quadVAO->Delete();
        if (quadVBO) quadVBO->Delete();
        if (rayMarchShader) rayMarchShader->Delete();
    }
    
    /**
     * Get reference to settings for UI modification
     */
    Settings& getSettings() { return settings; }
    
    /**
     * Get camera for external control
     */
    EndCamera* getCamera() { return camera.get(); }
    
private:
    /**
     * Create the fullscreen quad used for ray marching
     */
    bool createFullscreenQuad() {
        // Simple quad covering the screen (-1 to 1 in clip space)
        float quadVertices[] = {
            // Position (x, y)
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f
        };
        
        quadVAO = std::make_unique<VAO>();
        quadVAO->Bind();
        
        quadVBO = std::make_unique<VBO>(quadVertices, sizeof(quadVertices));
        
        // Position attribute (location 0)
        quadVAO->LinkAttrib(*quadVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
        
        quadVAO->Unbind();
        quadVBO->Unbind();
        
        return true;
    }
    
    /**
     * Load and compile the ray marching shaders
     */
    bool loadShaders() {
        try {
            rayMarchShader = std::make_unique<Shader>(
                "shaders/end_raymarch.vert",
                "shaders/end_raymarch.frag"
            );
            return true;
        } catch (const std::exception& e) {
            std::cerr << "EndRenderer: Shader loading failed: " << e.what() << std::endl;
            return false;
        } catch (int errnum) {
            std::cerr << "EndRenderer: Shader file not found, errno: " << errnum << std::endl;
            return false;
        }
    }
    
    /**
     * Set all shader uniforms for current frame
     */
    void setShaderUniforms(int octaves, float stepMult) {
        GLuint program = rayMarchShader->ID;
        
        // Camera uniforms
        glUniform3fv(glGetUniformLocation(program, "uCameraPos"), 1, 
                     glm::value_ptr(camera->localOffset));
        glUniform3iv(glGetUniformLocation(program, "uChunkOrigin"), 1, 
                     glm::value_ptr(camera->chunkOrigin));
        glUniform1f(glGetUniformLocation(program, "uCameraAltitude"), 
                    static_cast<float>(camera->getAltitude()));
        
        // View matrices
        glm::mat4 invViewProj = camera->getInverseViewProjection();
        glUniformMatrix4fv(glGetUniformLocation(program, "uInvViewProj"), 1, GL_FALSE,
                          glm::value_ptr(invViewProj));
        
        // Rendering settings
        glUniform1f(glGetUniformLocation(program, "uMaxDistance"), settings.maxDistance);
        glUniform1i(glGetUniformLocation(program, "uMaxSteps"), settings.maxSteps);
        glUniform1f(glGetUniformLocation(program, "uTime"), 
                    static_cast<float>(glfwGetTime()));
        
        // Quality settings
        glUniform1i(glGetUniformLocation(program, "uOctaves"), octaves);
        glUniform1f(glGetUniformLocation(program, "uStepMultiplier"), stepMult);
        
        // Colors
        glUniform3fv(glGetUniformLocation(program, "uEndStoneColor"), 1,
                     glm::value_ptr(settings.endStoneColor));
        glUniform3fv(glGetUniformLocation(program, "uSkyColor"), 1,
                     glm::value_ptr(settings.skyColor));
        glUniform3fv(glGetUniformLocation(program, "uFogColor"), 1,
                     glm::value_ptr(settings.fogColor));
        glUniform1f(glGetUniformLocation(program, "uFogDensity"), settings.fogDensity);
    }
    
    /**
     * Render ImGui debug interface
     */
    void renderDebugUI() {
        ImGui::Begin("End Dimension Viewer");
        
        // Performance
        ImGui::Text("Performance");
        ImGui::Text("FPS: %.1f (%.2f ms/frame)", averageFPS, 1000.0f / averageFPS);
        ImGui::Separator();
        
        // Camera info
        ImGui::Text("Camera");
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", 
                    camera->position.x, camera->position.y, camera->position.z);
        ImGui::Text("Chunk: (%d, %d, %d)", 
                    camera->chunkOrigin.x, camera->chunkOrigin.y, camera->chunkOrigin.z);
        ImGui::Text("Distance from origin: %.1f blocks", camera->getDistanceFromOrigin());
        ImGui::Text("LOD Factor: %.2f", camera->getLODFactor());
        ImGui::Text("Current Speed: %.1f blocks/s", camera->getCurrentSpeed());
        
        ImGui::SliderFloat("Speed Multiplier", &camera->speedMultiplier, 0.1f, 100.0f, "%.1f", 
                          ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Mouse Sensitivity", &camera->sensitivity, 0.01f, 1.0f);
        
        // Camera mode
        const char* modes[] = { "Free Flight", "Orbit", "Cinematic" };
        int currentMode = static_cast<int>(camera->mode);
        if (ImGui::Combo("Camera Mode", &currentMode, modes, 3)) {
            camera->setMode(static_cast<EndCamera::Mode>(currentMode));
        }
        
        if (ImGui::Button("Reset Camera")) {
            camera->reset();
        }
        ImGui::SameLine();
        if (ImGui::Button("Go to Ring")) {
            camera->teleportTo(glm::dvec3(3000.0, 200.0, 0.0));
        }
        
        ImGui::Separator();
        
        // Quality settings
        ImGui::Text("Quality Settings");
        ImGui::SliderInt("Max Steps", &settings.maxSteps, 32, 512);
        ImGui::SliderFloat("Max Distance", &settings.maxDistance, 1000.0f, 100000.0f, "%.0f",
                          ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Step Multiplier", &settings.stepMultiplier, 0.1f, 4.0f);
        ImGui::SliderInt("Base Octaves", &settings.baseOctaves, 1, 6);
        
        ImGui::Separator();
        
        // Colors
        ImGui::Text("Colors");
        ImGui::ColorEdit3("End Stone", glm::value_ptr(settings.endStoneColor));
        ImGui::ColorEdit3("Sky", glm::value_ptr(settings.skyColor));
        ImGui::ColorEdit3("Fog", glm::value_ptr(settings.fogColor));
        ImGui::SliderFloat("Fog Density", &settings.fogDensity, 0.0f, 5.0f);
        
        ImGui::Separator();
        
        // Teleport locations
        ImGui::Text("Quick Teleports");
        if (ImGui::Button("Main Island")) {
            camera->teleportTo(glm::dvec3(0.0, 100.0, 200.0));
        }
        ImGui::SameLine();
        if (ImGui::Button("Exclusion Zone")) {
            camera->teleportTo(glm::dvec3(800.0, 100.0, 0.0));
        }
        if (ImGui::Button("Outer Islands")) {
            camera->teleportTo(glm::dvec3(2000.0, 100.0, 0.0));
        }
        ImGui::SameLine();
        if (ImGui::Button("Ring View")) {
            camera->teleportTo(glm::dvec3(0.0, 5000.0, 0.0));
            camera->orientation = glm::dvec3(0.0, -1.0, 0.0);
        }
        if (ImGui::Button("Cosmic View")) {
            camera->teleportTo(glm::dvec3(0.0, 50000.0, 0.0));
            camera->orientation = glm::dvec3(0.0, -1.0, 0.0);
        }
        
        ImGui::Separator();
        
        // CPU verification
        if (ImGui::CollapsingHeader("CPU Density Test")) {
            static float testCoord[3] = {0.0f, 64.0f, 0.0f};
            ImGui::InputFloat3("Test Coordinate", testCoord);
            
            if (ImGui::Button("Sample Density")) {
                double density = cpuDensity->sample(testCoord[0], testCoord[1], testCoord[2]);
                ImGui::Text("Density at (%.1f, %.1f, %.1f): %.4f %s", 
                           testCoord[0], testCoord[1], testCoord[2],
                           density, density > 0 ? "(SOLID)" : "(AIR)");
            }
        }
        
        ImGui::End();
        
        // Controls help window
        ImGui::Begin("Controls");
        ImGui::Text("WASD - Move horizontally");
        ImGui::Text("Space/Shift - Move up/down");
        ImGui::Text("Right Mouse + Drag - Look around");
        ImGui::Text("+/- - Adjust speed");
        ImGui::Text("F - Reset to origin");
        ImGui::Text("Tab - Toggle UI");
        ImGui::End();
    }
    
    /**
     * Update FPS and performance metrics
     */
    void updatePerformanceMetrics(float deltaTime) {
        frameTimeAccum += deltaTime;
        frameCount++;
        
        if (frameTimeAccum >= 0.5f) {  // Update every 0.5 seconds
            averageFPS = frameCount / frameTimeAccum;
            frameTimeAccum = 0.0f;
            frameCount = 0;
        }
    }
    
    /**
     * Verify density function against known Minecraft coordinates
     */
    void verifyDensityFunction() {
        std::cout << "EndRenderer: Verifying density function..." << std::endl;
        
        struct TestCase {
            double x, y, z;
            const char* location;
            bool expectSolid;
        };
        
        TestCase tests[] = {
            {0.0, 64.0, 0.0, "Main island center", true},
            {0.0, 200.0, 0.0, "High above main island", false},
            {0.0, 0.0, 0.0, "Deep void", false},
            {500.0, 64.0, 500.0, "Exclusion zone", false},
            {3000.0, 64.0, 0.0, "Ring sweet spot", true},  // Should have islands nearby
        };
        
        for (const auto& test : tests) {
            double density = cpuDensity->sample(test.x, test.y, test.z);
            bool isSolid = density > 0.0;
            const char* result = (isSolid == test.expectSolid) ? "PASS" : "UNEXPECTED";
            
            std::cout << "  " << test.location << " (" << test.x << ", " << test.y << ", " << test.z << "): "
                      << "density=" << density << " (" << (isSolid ? "solid" : "air") << ") - " 
                      << result << std::endl;
        }
    }
};

} // namespace EndViewer

#endif // END_RENDERER_H
