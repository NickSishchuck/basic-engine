// In Engine/Common/include/RendererInterface.h
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

    // Cube rendering - updated to support full transformation matrix
    virtual void CreateCube() = 0;
    virtual void RenderCube(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) = 0;
    virtual void RenderCube(const glm::mat4& transformMatrix) = 0;  // New method

    // Floor rendering
    virtual void CreateFloor() = 0;
    virtual void RenderFloor(float size = 20.0f, int gridLines = 20) = 0;
    virtual void SetFloorEnabled(bool enabled) = 0;
    virtual bool IsFloorEnabled() const = 0;


    // 2D Rendering capabilities
    virtual void BeginRender2D() = 0;
    virtual void EndRender2D() = 0;

    // 2D Shape rendering
    virtual void CreateCircle() = 0;
    virtual void RenderCircle2D(const glm::vec2& position, float radius, const glm::vec3& color = glm::vec3(1.0f)) = 0;
    virtual void RenderRect2D(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color = glm::vec3(1.0f)) = 0;

    // Batch rendering for particles
    virtual void BeginBatch() = 0;
    virtual void AddCircleToBatch(const glm::vec2& position, float radius, const glm::vec3& color) = 0;
    virtual void RenderBatch() = 0;
    virtual void EndBatch() = 0;
};

} // namespace Common
} // namespace Engine
#endif
