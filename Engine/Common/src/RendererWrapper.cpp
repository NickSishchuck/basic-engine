#include "../include/RendererWrapper.h"
#include "../../../renderer/include/shaderClass.h"
#include "../../../renderer/include/VAO.h"
#include "../../../renderer/include/VBO.h"
#include "../../../renderer/include/EBO.h"
#include "../../../renderer/include/ImGuiManager.h"
#include "../../../renderer/include/Camera.h"
#include <iostream>

namespace Engine {
namespace Common {

OpenGLRendererWrapper::OpenGLRendererWrapper() : window(nullptr) {
    // Triangle vertices - copied from original renderer
    static GLfloat verticesData[] = {
        -0.5f,  0.0f,  0.5f,     1.0f, 0.0f, 0.0f,  // Lower left
        -0.5f,  0.0f, -0.5f,     0.0f, 1.0f, 0.0f,  // Upper left
         0.5f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,  // Upper right
         0.5f,  0.0f,  0.5f,     1.0f, 1.0f, 1.0f,  // Lower right
         0.0f,  0.8f,  0.0f,     1.0f, 1.0f, 0.0f   // Top
    };
    vertices = verticesData;
    verticesSize = sizeof(verticesData);

    static GLuint indicesData[] = {
        0, 1, 2,
        0, 2, 3,
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };
    indices = indicesData;
    indicesSize = sizeof(indicesData);
}

OpenGLRendererWrapper::~OpenGLRendererWrapper() {
    Shutdown();
}

bool OpenGLRendererWrapper::Initialize(int width, int height, const char* title) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (GLenum err = glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return false;
    }

    // Setup shader
    shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");

    // Setup VAO, VBO, EBO - similar to main.cpp in renderer
    vao = std::make_unique<VAO>();
    vao->Bind();

    vbo = std::make_unique<VBO>(vertices, verticesSize);
    ebo = std::make_unique<EBO>(indices, indicesSize);

    vao->LinkAttrib(*vbo.get(), 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    vao->LinkAttrib(*vbo.get(), 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    vao->Unbind();
    vbo->Unbind();
    ebo->Unbind();

    // Setup ImGui
    imguiManager = std::make_unique<ImGuiManager>(window);
    imguiManager->Initialize();

    // Setup camera
    camera = std::make_unique<Camera>(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    return true;
}

void OpenGLRendererWrapper::BeginFrame() {
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the pyramid
    shader->Activate();

    // Update camera
    camera->Inputs(window);
    camera->Matrix(45.0f, 0.1f, 100.0f, *shader.get(), "camMatrix");

    vao->Bind();
    glDrawElements(GL_TRIANGLES, indicesSize/sizeof(unsigned int), GL_UNSIGNED_INT, 0);

    // Begin ImGui frame
    imguiManager->BeginFrame();

    // Create a simple ImGui window
    ImGui::Begin("Renderer Controls");
    ImGui::Text("Hello from the OpenGLRendererWrapper!");
    ImGui::SliderFloat("Camera Speed", &camera->speed, 0.01f, 0.2f);
    ImGui::SliderFloat("Camera Sensitivity", &camera->sensitivity, 10.0f, 100.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();
}

void OpenGLRendererWrapper::EndFrame() {
    // Finish ImGui rendering
    imguiManager->EndFrame();
    imguiManager->Render();

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void OpenGLRendererWrapper::Shutdown() {
    if (imguiManager) {
        imguiManager->Shutdown();
    }

    if (vao) vao->Delete();
    if (vbo) vbo->Delete();
    if (ebo) ebo->Delete();
    if (shader) shader->Delete();

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

} // namespace Common
} // namespace Engine
