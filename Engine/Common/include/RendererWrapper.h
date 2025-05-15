#ifndef OPENGL_RENDERER_WRAPPER_H
#define OPENGL_RENDERER_WRAPPER_H

#include "RendererInterface.h"

namespace Engine {
namespace Common {

class OpenGLRendererWrapper : public RendererInterface {
private:
    GLFWwindow* window;

public:
    OpenGLRendererWrapper();
    ~OpenGLRendererWrapper() override;

    bool Initialize(int width, int height, const char* title) override;
    void BeginFrame() override;
    void EndFrame() override;
    void Shutdown() override;
    GLFWwindow* GetWindow() const override { return window; }

    // Basic rendering method to test integration
    void RenderTriangle();
};

} // namespace Common
} // namespace Engine
#endif
