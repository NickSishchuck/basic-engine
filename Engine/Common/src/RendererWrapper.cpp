#include "../include/RendererWrapper.h"
#include "../../../renderer/include/shaderClass.h"
#include "../../../renderer/include/VAO.h"
#include "../../../renderer/include/VBO.h"
#include "../../../renderer/include/EBO.h"
#include "../../../renderer/include/ImGuiManager.h"
#include "../../../renderer/include/Camera.h"
#include "../../../renderer/include/Logger.h"
#include <iostream>
#include <vector>

namespace Engine {
namespace Common {

OpenGLRendererWrapper::OpenGLRendererWrapper() : window(nullptr), floorEnabled(true), floorSize(20.0f), gridLineCount(20), autoUpdateFloor(true), lastFloorSize(20.0f), lastGridLineCount(20) {
    // Triangle vertices
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

    CreateCube();
    CreateFloor();

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

    // Current time for animation
    static float lastFrameTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    // Update camera
    camera->Inputs(window);

    // ----- RENDER FLOOR ------
    if (floorEnabled) {
        RenderFloor(floorSize, gridLineCount);
    }

    // ----- RENDER CUBE ------
    // Animate the cube position (left-right movement)
    static float direction = 1.0f; // 1 for right, -1 for left
    static float speed = 1.0f;     // Units per second

    // Update cube position
    cubePosition.x += direction * speed * deltaTime;

    // Reverse direction when reaching boundaries
    if (cubePosition.x > 2.0f) {
        direction = -1.0f;
    }
    else if (cubePosition.x < -2.0f) {
        direction = 1.0f;
    }

    glm::vec3 renderPosition = cubePosition;
    renderPosition.z = -3.0f;
    renderPosition.y = 1.0f;

    glm::vec3 cubeScale(0.8f);

    RenderCube(renderPosition, cubeScale);

    imguiManager->BeginFrame();

    ImGui::Begin("Renderer Controls");
    ImGui::Text("Hello from the OpenGLRendererWrapper!");

    // Floor controls
    ImGui::Separator();
    ImGui::Text("Floor Settings");
    if (ImGui::Checkbox("Enable Floor", &floorEnabled)) {
        // Floor enabled/disabled - no additional action needed
    }
    if (floorEnabled) {
        ImGui::Checkbox("Auto-Update Floor", &autoUpdateFloor);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("When enabled, floor updates instantly as you drag sliders");
        }

        bool settingsChanged = false;

        if (ImGui::SliderFloat("Floor Size", &floorSize, 5.0f, 50.0f)) {
            if (floorSize != lastFloorSize) {
                settingsChanged = true;
            }
        }

        if (ImGui::SliderInt("Grid Lines", &gridLineCount, 5, 50)) {
            if (gridLineCount != lastGridLineCount) {
                settingsChanged = true;
            }
        }

        // Auto-update if enabled and settings changed
        if (autoUpdateFloor && settingsChanged) {
            CreateFloor();
            lastFloorSize = floorSize;
            lastGridLineCount = gridLineCount;
        }

        // Manual regenerate button
        if (ImGui::Button("Regenerate Floor")) {
            CreateFloor();
            lastFloorSize = floorSize;
            lastGridLineCount = gridLineCount;
        }
        if (!autoUpdateFloor) {
            ImGui::SameLine();
            ImGui::TextDisabled("Manual mode - use button to apply changes");
        }
    }

    // Camera controls
    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::SliderFloat("Camera Speed", &camera->speed, 0.01f, 0.2f);
    ImGui::SliderFloat("Camera Sensitivity", &camera->sensitivity, 10.0f, 100.0f);

    // Cube controls
    ImGui::Separator();
    ImGui::Text("Cube Settings");
    ImGui::SliderFloat("Cube Speed", &speed, 0.1f, 3.0f);
    ImGui::Text("Cube Position: %.2f, %.2f, %.2f", renderPosition.x, renderPosition.y, renderPosition.z);

    // Performance
    ImGui::Separator();
    ImGui::Text("Performance");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();
}

void OpenGLRendererWrapper::EndFrame() {
    imguiManager->EndFrame();
    imguiManager->Render();

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

    if (cubeVAO) cubeVAO->Delete();
    if (cubeVBO) cubeVBO->Delete();
    if (cubeEBO) cubeEBO->Delete();

    if (floorVAO) floorVAO->Delete();
    if (floorVBO) floorVBO->Delete();
    if (floorEBO) floorEBO->Delete();

    if (gridVAO) gridVAO->Delete();
    if (gridVBO) gridVBO->Delete();

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void OpenGLRendererWrapper::CreateCube() {
    // Define cube vertices (position and color)
    static GLfloat cubeVertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,     1.0f, 0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f,  0.5f,     1.0f, 1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f,  0.5f,     1.0f, 1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f,  0.5f,     1.0f, 0.0f, 1.0f,  // Top-left

        // Back face
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,  // Bottom-left
         0.5f, -0.5f, -0.5f,     0.0f, 1.0f, 1.0f,  // Bottom-right
         0.5f,  0.5f, -0.5f,     0.0f, 1.0f, 0.0f,  // Top-right
        -0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f   // Top-left
    };

    // Define cube indices
    static GLuint cubeIndices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        // Right face
        1, 5, 6,
        6, 2, 1,
        // Back face
        5, 4, 7,
        7, 6, 5,
        // Left face
        4, 0, 3,
        3, 7, 4,
        // Top face
        3, 2, 6,
        6, 7, 3,
        // Bottom face
        4, 5, 1,
        1, 0, 4
    };

    // Create buffers for the cube
    cubeVAO = std::make_unique<VAO>();
    cubeVAO->Bind();

    cubeVBO = std::make_unique<VBO>(cubeVertices, sizeof(cubeVertices));
    cubeEBO = std::make_unique<EBO>(cubeIndices, sizeof(cubeIndices));

    cubeVAO->LinkAttrib(*cubeVBO.get(), 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    cubeVAO->LinkAttrib(*cubeVBO.get(), 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    cubeVAO->Unbind();
    cubeVBO->Unbind();
    cubeEBO->Unbind();

    // Initial position
    cubePosition = glm::vec3(0.0f, 0.0f, 0.0f);
}

void OpenGLRendererWrapper::RenderCube(const glm::vec3& position, const glm::vec3& scale) {
    if (!cubeVAO) {
        CreateCube();
    }

    // Activate shader
    shader->Activate();

    // Create model matrix for the cube
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    // Set the camera matrix
    camera->Matrix(45.0f, 0.1f, 100.0f, *shader.get(), "camMatrix");

    // Set the model matrix uniform
    GLuint modelLoc = glGetUniformLocation(shader->ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Switch to cube VAO
    cubeVAO->Bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    cubeVAO->Unbind();
}

void OpenGLRendererWrapper::CreateFloor() {
    CreateFloorPlane(floorSize);
    CreateGridLines(floorSize, gridLineCount);
}

void OpenGLRendererWrapper::CreateFloorPlane(float size) {
    // Create a large plane at y = 0
    float halfSize = size / 2.0f;

    GLfloat floorVertices[] = {
        // Position                    // Color (dark gray)
        -halfSize, 0.0f, -halfSize,    0.3f, 0.3f, 0.3f,  // Bottom-left
         halfSize, 0.0f, -halfSize,    0.3f, 0.3f, 0.3f,  // Bottom-right
         halfSize, 0.0f,  halfSize,    0.3f, 0.3f, 0.3f,  // Top-right
        -halfSize, 0.0f,  halfSize,    0.3f, 0.3f, 0.3f   // Top-left
    };

    GLuint floorIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create buffers for the floor
    floorVAO = std::make_unique<VAO>();
    floorVAO->Bind();

    floorVBO = std::make_unique<VBO>(floorVertices, sizeof(floorVertices));
    floorEBO = std::make_unique<EBO>(floorIndices, sizeof(floorIndices));

    floorVAO->LinkAttrib(*floorVBO.get(), 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    floorVAO->LinkAttrib(*floorVBO.get(), 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    floorVAO->Unbind();
    floorVBO->Unbind();
    floorEBO->Unbind();
}

void OpenGLRendererWrapper::CreateGridLines(float size, int gridLines) {
    std::vector<GLfloat> gridVertices;
    float halfSize = size / 2.0f;
    float step = size / gridLines;

    // Grid line color (light gray)
    float r = 0.5f, g = 0.5f, b = 0.5f;

    // Vertical lines (parallel to Z axis)
    for (int i = 0; i <= gridLines; ++i) {
        float x = -halfSize + i * step;

        // Line start
        gridVertices.push_back(x);
        gridVertices.push_back(0.0f);
        gridVertices.push_back(-halfSize);
        gridVertices.push_back(r);
        gridVertices.push_back(g);
        gridVertices.push_back(b);

        // Line end
        gridVertices.push_back(x);
        gridVertices.push_back(0.0f);
        gridVertices.push_back(halfSize);
        gridVertices.push_back(r);
        gridVertices.push_back(g);
        gridVertices.push_back(b);
    }

    // Horizontal lines (parallel to X axis)
    for (int i = 0; i <= gridLines; ++i) {
        float z = -halfSize + i * step;

        // Line start
        gridVertices.push_back(-halfSize);
        gridVertices.push_back(0.0f);
        gridVertices.push_back(z);
        gridVertices.push_back(r);
        gridVertices.push_back(g);
        gridVertices.push_back(b);

        // Line end
        gridVertices.push_back(halfSize);
        gridVertices.push_back(0.0f);
        gridVertices.push_back(z);
        gridVertices.push_back(r);
        gridVertices.push_back(g);
        gridVertices.push_back(b);
    }

    // Create VAO and VBO for grid lines
    gridVAO = std::make_unique<VAO>();
    gridVAO->Bind();

    gridVBO = std::make_unique<VBO>(gridVertices.data(), gridVertices.size() * sizeof(GLfloat));

    gridVAO->LinkAttrib(*gridVBO.get(), 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    gridVAO->LinkAttrib(*gridVBO.get(), 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    gridVAO->Unbind();
    gridVBO->Unbind();
}

void OpenGLRendererWrapper::RenderFloor(float size, int gridLines) {
    if (!floorVAO || !gridVAO) {
        CreateFloor();
    }

    // Activate shader
    shader->Activate();

    // Set camera matrix
    camera->Matrix(45.0f, 0.1f, 100.0f, *shader.get(), "camMatrix");

    // Identity model matrix for floor (no transformation needed)
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    GLuint modelLoc = glGetUniformLocation(shader->ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Render floor plane
    floorVAO->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    floorVAO->Unbind();

    // Render grid lines
    gridVAO->Bind();
    glDrawArrays(GL_LINES, 0, (gridLines + 1) * 4); // 2 points per line, (gridLines+1) lines in each direction
    gridVAO->Unbind();
}

} // namespace Common
} // namespace Engine
