#ifndef RENDERER_INTERFACE_H
#define RENDERER_INTERFACE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Engine {
namespace Common {

class RendererInterface {
public:
    virtual ~RendererInterface() = default;
    virtual bool Initialize(int width, int height, const char* title) = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
    virtual GLFWwindow* GetWindow() const = 0;
};

} // namespace Common
} // namespace Engine
#endif
