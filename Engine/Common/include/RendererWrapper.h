#ifndef OPENGL_RENDERER_WRAPPER_H
#define OPENGL_RENDERER_WRAPPER_H

#include "../../../renderer/include/FrameBuffer.h"
#include "RendererInterface.h"
#include <memory>
#include <vector>

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
  GLFWwindow *window;

  // Store references to renderer components
  std::unique_ptr<Shader> shader;
  std::unique_ptr<VAO> vao;
  std::unique_ptr<VBO> vbo;
  std::unique_ptr<EBO> ebo;
  std::unique_ptr<ImGuiManager> imguiManager;
  std::unique_ptr<Camera> camera;

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

  // std::unique_ptr<VAO> rectVAO;
  // std::unique_ptr<VBO> rectVBO;
  // std::unique_ptr<EBO> rectEBO;

  bool floorEnabled;
  float floorSize;
  int gridLineCount;
  bool autoUpdateFloor;

  float lastFloorSize;
  int lastGridLineCount;

  float *vertices;
  unsigned int *indices;
  int verticesSize;
  int indicesSize;

public:
  OpenGLRendererWrapper();
  ~OpenGLRendererWrapper() override;

  bool Initialize(int width, int height, const char *title) override;
  void BeginFrame() override;
  void EndFrame() override;
  void Shutdown() override;
  GLFWwindow *GetWindow() const override { return window; }

  void CreateCube() override;
  void RenderCube(const glm::vec3 &position,
                  const glm::vec3 &scale = glm::vec3(1.0f)) override;
  void RenderCube(const glm::mat4 &transformMatrix) override;

  void CreateFloor() override;
  void RenderFloor(float size = 20.0f, int gridLines = 20) override;
  void SetFloorEnabled(bool enabled) override { floorEnabled = enabled; }
  bool IsFloorEnabled() const override { return floorEnabled; }

  void BeginViewportRender();
  void EndViewportRender();
  GLuint GetViewportTexture() const;
  void ResizeViewport(int width, int height);
  void SetMainWindowSize(int width, int height);

private:
  void CreateFloorPlane(float size);
  void CreateGridLines(float size, int gridLines);
};

} // namespace Common
} // namespace Engine
#endif
