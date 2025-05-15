
#ifndef OPENGL_RENDERER_WRAPPER_H
#define OPENGL_RENDERER_WRAPPER_H

#include "RendererInterface.h"
#include <memory>

// Forward declarations for renderer components
class Shader;
class VAO;
class VBO;
class EBO;
class ImGuiManager;
class Camera;

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
};

} // namespace Common
} // namespace Engine
#endif
