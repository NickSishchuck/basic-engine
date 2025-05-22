#ifndef RENDERER_INTERFACE_H
#define RENDERER_INTERFACE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    // Cube rendering
    virtual void CreateCube() = 0;
    virtual void RenderCube(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) = 0;

    // Floor rendering
    virtual void CreateFloor() = 0;
    virtual void RenderFloor(float size = 20.0f, int gridLines = 20) = 0;
    virtual void SetFloorEnabled(bool enabled) = 0;
    virtual bool IsFloorEnabled() const = 0;
};

} // namespace Common
} // namespace Engine
#endif
