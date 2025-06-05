#ifndef OPENGL_RENDERER_WRAPPER_H
#define OPENGL_RENDERER_WRAPPER_H

#include "RendererInterface.h"
#include "../../../renderer/include/FrameBuffer.h"
#include <vector>
#include <memory>

// Forward declarations for renderer components
class Shader;
class VAO;
class VBO;
class EBO;
class ImGuiManager;
class Camera;
class Camera2D;  // New 2D camera

namespace Engine {
namespace Common {

class OpenGLRendererWrapper : public RendererInterface {

    private:
        GLFWwindow* window;

        // Store references to renderer components
        std::unique_ptr<Shader> shader;
        std::unique_ptr<VAO> vao;
        std::unique_ptr<VBO> vbo;
        std::unique_ptr<EBO> ebo;
        std::unique_ptr<ImGuiManager> imguiManager;
        std::unique_ptr<Camera> camera;

        // 2D Rendering components
        std::unique_ptr<Camera2D> camera2D;
        std::unique_ptr<Shader> shader2D;
        bool rendering2D = false;

        // Viewport rendering
        std::unique_ptr<Framebuffer> viewportFramebuffer;
        bool isRenderingToViewport = false;
        int viewportWidth = 800;
        int viewportHeight = 600;

        // For tracking main window size
        int windowWidth;
        int windowHeight;


        // For cube rendering
        std::unique_ptr<VAO> cubeVAO;
        std::unique_ptr<VBO> cubeVBO;
        std::unique_ptr<EBO> cubeEBO;

        // For floor rendering
        std::unique_ptr<VAO> floorVAO;
        std::unique_ptr<VBO> floorVBO;
        std::unique_ptr<EBO> floorEBO;

        // For grid lines
        std::unique_ptr<VAO> gridVAO;
        std::unique_ptr<VBO> gridVBO;

        // 2D Shape rendering
        std::unique_ptr<VAO> circleVAO;
        std::unique_ptr<VBO> circleVBO;
        std::unique_ptr<EBO> circleEBO;

        std::unique_ptr<VAO> rectVAO;
        std::unique_ptr<VBO> rectVBO;
        std::unique_ptr<EBO> rectEBO;

        // Batch rendering for particles
        struct ParticleBatch {
            std::vector<float> vertices;
            std::vector<unsigned int> indices;
            std::unique_ptr<VAO> batchVAO;
            std::unique_ptr<VBO> batchVBO;
            std::unique_ptr<EBO> batchEBO;
                size_t particleCount = 0;
            } particleBatch;
        // Floor settings
        bool floorEnabled;
        float floorSize;
        int gridLineCount;
        bool autoUpdateFloor;

        // For tracking changes
        float lastFloorSize;
        int lastGridLineCount;

        // Original vertices and indices from the renderer
        float* vertices;
        unsigned int* indices;
        int verticesSize;
        int indicesSize;

public:
    OpenGLRendererWrapper();
    ~OpenGLRendererWrapper() override;

    bool Initialize(int width, int height, const char* title) override;
    void BeginFrame() override;
    void EndFrame() override;
    void Shutdown() override;
    GLFWwindow* GetWindow() const override { return window; }

    void CreateCube() override;
    void RenderCube(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) override;
    void RenderCube(const glm::mat4& transformMatrix) override;


    void CreateFloor() override;
    void RenderFloor(float size = 20.0f, int gridLines = 20) override;
    void SetFloorEnabled(bool enabled) override { floorEnabled = enabled; }
    bool IsFloorEnabled() const override { return floorEnabled; }


    // 2D Rendering capabilities
    void BeginRender2D() override;
    void EndRender2D() override;

    // 2D Shape rendering
    void CreateCircle() override;
    void RenderCircle2D(const glm::vec2& position, float radius, const glm::vec3& color = glm::vec3(1.0f)) override;
    void RenderRect2D(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color = glm::vec3(1.0f)) override;

    // Batch rendering for particles
    void BeginBatch() override;
    void AddCircleToBatch(const glm::vec2& position, float radius, const glm::vec3& color) override;
    void RenderBatch() override;
    void EndBatch() override;

    // Viewport rendering methods
    void BeginViewportRender();
    void EndViewportRender();
    GLuint GetViewportTexture() const;
    void ResizeViewport(int width, int height);
    void SetMainWindowSize(int width, int height);

private:
    void CreateFloorPlane(float size);
    void CreateGridLines(float size, int gridLines);

    // 2D shape creation helpers
    void CreateCircleGeometry();
    void CreateRectGeometry();
    void InitializeBatchSystem();
};

} // namespace Common
} // namespace Engine
#endif
