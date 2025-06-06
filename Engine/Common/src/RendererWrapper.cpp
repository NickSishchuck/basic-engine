#include "../include/RendererWrapper.h"
#include "../../../renderer/include/shaderClass.h"
#include "../../../renderer/include/VAO.h"
#include "../../../renderer/include/VBO.h"
#include "../../../renderer/include/EBO.h"
#include "../../../renderer/include/ImGuiManager.h"
#include "../../../renderer/include/Camera.h"
#include "../../../renderer/include/Camera2D.h"
#include "../../../renderer/include/Logger.h"
#include <filesystem>
#include <iostream>
#include <vector>

namespace Engine {
namespace Common {

    OpenGLRendererWrapper::OpenGLRendererWrapper() :
        window(nullptr),
        floorEnabled(true),
        floorSize(20.0f),
        gridLineCount(20),
        autoUpdateFloor(true),
        lastFloorSize(20.0f),
        lastGridLineCount(20),
        vertices(nullptr),
        indices(nullptr),
        verticesSize(0),
        indicesSize(0),
        windowWidth(800),
        windowHeight(600),
        viewportWidth(800),
        viewportHeight(600),
        isRenderingToViewport(false),
        rendering2D(false) {  // Initialize 2D rendering flag

        std::cout << "DEBUG: OpenGLRendererWrapper constructor called" << std::endl;

        // Triangle vertices - allocate on heap instead of using static
        verticesSize = 30 * sizeof(GLfloat);  // 5 vertices * 6 components each
        vertices = new float[30] {
            -0.5f,  0.0f,  0.5f,     1.0f, 0.0f, 0.0f,  // Lower left
            -0.5f,  0.0f, -0.5f,     0.0f, 1.0f, 0.0f,  // Upper left
             0.5f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,  // Upper right
             0.5f,  0.0f,  0.5f,     1.0f, 1.0f, 1.0f,  // Lower right
             0.0f,  0.8f,  0.0f,     1.0f, 1.0f, 0.0f   // Top
        };

        indicesSize = 18 * sizeof(GLuint);  // 6 triangles * 3 vertices each
        indices = new unsigned int[18] {
            0, 1, 2,
            0, 2, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4,
            3, 0, 4
        };

        std::cout << "DEBUG: OpenGLRendererWrapper constructor completed" << std::endl;
    }

    OpenGLRendererWrapper::~OpenGLRendererWrapper() {
        std::cout << "DEBUG: OpenGLRendererWrapper destructor called" << std::endl;

        // Clean up dynamically allocated arrays
        if (vertices) {
            delete[] vertices;
            vertices = nullptr;
        }
        if (indices) {
            delete[] indices;
            indices = nullptr;
        }

        Shutdown();
    }

bool OpenGLRendererWrapper::Initialize(int width, int height, const char* title) {
    std::cout << "DEBUG: Starting Initialize..." << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    std::cout << "DEBUG: GLFW initialized" << std::endl;

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    std::cout << "DEBUG: Creating window..." << std::endl;
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    std::cout << "DEBUG: Window created" << std::endl;

    // Store window dimensions
    windowWidth = width;
    windowHeight = height;

    // Make the window's context current
    glfwMakeContextCurrent(window);
    std::cout << "DEBUG: OpenGL context made current" << std::endl;

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (GLenum err = glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return false;
    }
    std::cout << "DEBUG: GLEW initialized" << std::endl;

    // Setup 2D camera and shader
        std::cout << "DEBUG: Creating 2D camera and shader..." << std::endl;
        camera2D = std::make_unique<Camera2D>(width, height, glm::vec2(0.0f, 0.0f));

        // Check if shader files exist
        std::cout << "DEBUG: Checking for 2D shader files..." << std::endl;
        std::ifstream vertFile("shaders/default2d.vert");
        std::ifstream fragFile("shaders/default2d.frag");

        if (!vertFile.good()) {
            std::cerr << "ERROR: shaders/default2d.vert not found!" << std::endl;
            std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
        }
        if (!fragFile.good()) {
            std::cerr << "ERROR: shaders/default2d.frag not found!" << std::endl;
        }

        vertFile.close();
        fragFile.close();

        try {
            shader2D = std::make_unique<Shader>("shaders/default2d.vert", "shaders/default2d.frag");
            std::cout << "DEBUG: 2D shaders loaded successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load 2D shaders: " << e.what() << std::endl;
            std::cerr << "2D rendering will be disabled" << std::endl;
            shader2D = nullptr;
        } catch (int errnum) {
            std::cerr << "Failed to load 2D shaders - errno: " << errnum << " - " << strerror(errnum) << std::endl;
            std::cerr << "Make sure 2D shader files exist at: shaders/default2d.vert and shaders/default2d.frag" << std::endl;
            shader2D = nullptr;
        }

        try {
                shader2D = std::make_unique<Shader>("shaders/default2d.vert", "shaders/default2d.frag");
                std::cout << "DEBUG: 2D shaders loaded successfully!" << std::endl;

                // Initialize 2D rendering components
                std::cout << "DEBUG: Initializing 2D rendering components..." << std::endl;
                CreateCircle();
                CreateRectGeometry();
                InitializeBatchSystem();
                std::cout << "DEBUG: 2D rendering components initialized!" << std::endl;

            } catch (const std::exception& e) {
                std::cerr << "Failed to load 2D shaders: " << e.what() << std::endl;
                std::cerr << "2D rendering will be disabled" << std::endl;
                shader2D = nullptr;
            }
    // Clear any GL errors from GLEW initialization
    while (glGetError() != GL_NO_ERROR);

    // Create viewport framebuffer (start with default size)
    std::cout << "DEBUG: Creating viewport framebuffer..." << std::endl;
    try {
        viewportFramebuffer = std::make_unique<Framebuffer>(viewportWidth, viewportHeight);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create framebuffer: " << e.what() << std::endl;
        return false;
    }
    std::cout << "DEBUG: Viewport framebuffer created" << std::endl;

    std::cout << "DEBUG: Creating cube..." << std::endl;
    CreateCube();
    std::cout << "DEBUG: Cube created" << std::endl;

    std::cout << "DEBUG: Creating floor..." << std::endl;
    CreateFloor();
    std::cout << "DEBUG: Floor created" << std::endl;

    // Setup shader
    std::cout << "DEBUG: Loading shaders..." << std::endl;
    try {
        shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shaders: " << e.what() << std::endl;
        return false;
    } catch (int errnum) {
        std::cerr << "Failed to load shaders - errno: " << errnum << " - " << strerror(errnum) << std::endl;
        std::cerr << "Make sure shader files exist at: shaders/default.vert and shaders/default.frag" << std::endl;
        return false;
    }
    std::cout << "DEBUG: Shaders loaded" << std::endl;

    // Setup VAO, VBO, EBO - similar to main.cpp in renderer
    std::cout << "DEBUG: Creating VAO, VBO, EBO..." << std::endl;
    vao = std::make_unique<VAO>();
    vao->Bind();

    vbo = std::make_unique<VBO>(vertices, verticesSize);
    ebo = std::make_unique<EBO>(indices, indicesSize);

    vao->LinkAttrib(*vbo.get(), 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    vao->LinkAttrib(*vbo.get(), 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    vao->Unbind();
    vbo->Unbind();
    ebo->Unbind();
    std::cout << "DEBUG: VAO, VBO, EBO created" << std::endl;

    // Setup ImGui
    std::cout << "DEBUG: Setting up ImGui..." << std::endl;
    imguiManager = std::make_unique<ImGuiManager>(window);
    imguiManager->Initialize();
    std::cout << "DEBUG: ImGui initialized" << std::endl;

    // Setup camera
    std::cout << "DEBUG: Creating camera..." << std::endl;
    camera = std::make_unique<Camera>(width, height, glm::vec3(0.0f, 3.0f, 10.0f));
    std::cout << "DEBUG: Camera created" << std::endl;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    std::cout << "DEBUG: Initialize completed successfully!" << std::endl;
    return true;
}

void OpenGLRendererWrapper::BeginFrame() {
    // Update window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    SetMainWindowSize(width, height);

    // Clear the main framebuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Current time for animation
    static float lastFrameTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    // Update camera
    camera->Inputs(window);

    // Start ImGui frame FIRST - this must be called before any ImGui functions!
    imguiManager->BeginFrame();

    // ----- RENDER FLOOR ------
    if (floorEnabled) {
        RenderFloor(floorSize, gridLineCount);
    }

    // Now we can safely use ImGui functions
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

    // Clean up 2D resources
    if (circleVAO) circleVAO->Delete();
    if (circleVBO) circleVBO->Delete();
    if (circleEBO) circleEBO->Delete();

    if (rectVAO) rectVAO->Delete();
    if (rectVBO) rectVBO->Delete();
    if (rectEBO) rectEBO->Delete();

    if (particleBatch.batchVAO) particleBatch.batchVAO->Delete();
    if (particleBatch.batchVBO) particleBatch.batchVBO->Delete();
    if (particleBatch.batchEBO) particleBatch.batchEBO->Delete();

        if (shader2D) shader2D->Delete();

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

void OpenGLRendererWrapper::RenderCube(const glm::mat4& transformMatrix) {
    if (!cubeVAO) {
        CreateCube();
    }

    // Activate shader
    shader->Activate();

    // Set the camera matrix
    camera->Matrix(45.0f, 0.1f, 100.0f, *shader.get(), "camMatrix");

    // Use the provided transformation matrix directly
    GLuint modelLoc = glGetUniformLocation(shader->ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transformMatrix));

    // Switch to cube VAO and render
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

void OpenGLRendererWrapper::BeginViewportRender() {
    isRenderingToViewport = true;
    viewportFramebuffer->Bind();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererWrapper::EndViewportRender() {
    viewportFramebuffer->Unbind();
    isRenderingToViewport = false;
    // Restore main window viewport
    glViewport(0, 0, windowWidth, windowHeight);
}

// 2D Rendering Implementation

void OpenGLRendererWrapper::BeginRender2D() {
    if (!shader2D || !camera2D) {
        std::cerr << "Warning: 2D rendering not available" << std::endl;
        return;
    }

    rendering2D = true;

    static bool first_time = true;
    if (first_time) {
        std::cout << "DEBUG: Beginning 2D rendering for first time" << std::endl;
        first_time = false;
    }

    // Switch to 2D rendering state
    glDisable(GL_DEPTH_TEST);  // Disable depth testing for 2D
    glEnable(GL_BLEND);        // Enable alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Activate 2D shader and set camera
    shader2D->Activate();
    camera2D->SetMatrices(*shader2D);
}

void OpenGLRendererWrapper::EndRender2D() {
    if (!rendering2D) return;

    rendering2D = false;

    // Restore 3D rendering state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void OpenGLRendererWrapper::CreateCircle() {
    CreateCircleGeometry();
}

void OpenGLRendererWrapper::CreateCircleGeometry() {
    // Create a quad that will be rendered as a circle in the fragment shader
    GLfloat circleVertices[] = {
        // Position    Color         TexCoord (for circle effect)
        -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, -1.0f,  // Bottom-left
         1.0f, -1.0f,  1.0f, 1.0f, 1.0f,   1.0f, -1.0f,  // Bottom-right
         1.0f,  1.0f,  1.0f, 1.0f, 1.0f,   1.0f,  1.0f,  // Top-right
        -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  -1.0f,  1.0f   // Top-left
    };

    GLuint circleIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    circleVAO = std::make_unique<VAO>();
    circleVAO->Bind();

    circleVBO = std::make_unique<VBO>(circleVertices, sizeof(circleVertices));
    circleEBO = std::make_unique<EBO>(circleIndices, sizeof(circleIndices));

    // Position attribute
    circleVAO->LinkAttrib(*circleVBO, 0, 2, GL_FLOAT, 7 * sizeof(float), (void*)0);
    // Color attribute
    circleVAO->LinkAttrib(*circleVBO, 1, 3, GL_FLOAT, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    // TexCoord attribute
    circleVAO->LinkAttrib(*circleVBO, 2, 2, GL_FLOAT, 7 * sizeof(float), (void*)(5 * sizeof(float)));

    circleVAO->Unbind();
    circleVBO->Unbind();
    circleEBO->Unbind();
}

void OpenGLRendererWrapper::RenderCircle2D(const glm::vec2& position, float radius, const glm::vec3& color) {
    if (!circleVAO || !shader2D || !rendering2D) return;

    // Create model matrix for the circle
    glm::mat3 modelMatrix = glm::mat3(
        radius, 0.0f, position.x,
        0.0f, radius, position.y,
        0.0f, 0.0f, 1.0f
    );

    // Set uniforms
    GLuint modelLoc = glGetUniformLocation(shader2D->ID, "model2D");
    glUniformMatrix3fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLuint shapeLoc = glGetUniformLocation(shader2D->ID, "shapeType");
    glUniform1i(shapeLoc, 1); // 1 = circle

    GLuint smoothLoc = glGetUniformLocation(shader2D->ID, "smoothness");
    glUniform1f(smoothLoc, 0.02f); // Smoothness for antialiasing

    // Override color if needed (this is a simple approach)
    // In a real implementation, you might want to pass color as a uniform

    circleVAO->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    circleVAO->Unbind();
}

void OpenGLRendererWrapper::CreateRectGeometry() {
    // Create a simple rectangle
    GLfloat rectVertices[] = {
        // Position    Color         TexCoord
        -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  // Bottom-left
         1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  // Bottom-right
         1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  // Top-right
        -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f   // Top-left
    };

    GLuint rectIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    rectVAO = std::make_unique<VAO>();
    rectVAO->Bind();

    rectVBO = std::make_unique<VBO>(rectVertices, sizeof(rectVertices));
    rectEBO = std::make_unique<EBO>(rectIndices, sizeof(rectIndices));

    // Position attribute
    rectVAO->LinkAttrib(*rectVBO, 0, 2, GL_FLOAT, 7 * sizeof(float), (void*)0);
    // Color attribute
    rectVAO->LinkAttrib(*rectVBO, 1, 3, GL_FLOAT, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    // TexCoord attribute
    rectVAO->LinkAttrib(*rectVBO, 2, 2, GL_FLOAT, 7 * sizeof(float), (void*)(5 * sizeof(float)));

    rectVAO->Unbind();
    rectVBO->Unbind();
    rectEBO->Unbind();
}

void OpenGLRendererWrapper::RenderRect2D(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color) {
    if (!rectVAO || !shader2D || !rendering2D) return;

    // Create model matrix for the rectangle
    glm::mat3 modelMatrix = glm::mat3(
        size.x / 2.0f, 0.0f, position.x,
        0.0f, size.y / 2.0f, position.y,
        0.0f, 0.0f, 1.0f
    );

    // Set uniforms
    GLuint modelLoc = glGetUniformLocation(shader2D->ID, "model2D");
    glUniformMatrix3fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLuint shapeLoc = glGetUniformLocation(shader2D->ID, "shapeType");
    glUniform1i(shapeLoc, 0); // 0 = rectangle

    rectVAO->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    rectVAO->Unbind();
}

// Batch Rendering Implementation

void OpenGLRendererWrapper::InitializeBatchSystem() {
    // Pre-allocate space for batch rendering
    particleBatch.vertices.reserve(10000 * 7); // 10000 particles * 7 floats per vertex
    particleBatch.indices.reserve(10000 * 6);  // 10000 particles * 6 indices per quad

    // Create VAO/VBO/EBO for batch rendering
    particleBatch.batchVAO = std::make_unique<VAO>();
    particleBatch.batchVBO = std::make_unique<VBO>(nullptr, 10000 * 4 * 7 * sizeof(float)); // Dynamic buffer
    particleBatch.batchEBO = std::make_unique<EBO>(nullptr, 10000 * 6 * sizeof(unsigned int)); // Dynamic buffer

    particleBatch.batchVAO->Bind();

    // Set up vertex attributes for batch rendering
    particleBatch.batchVAO->LinkAttrib(*particleBatch.batchVBO, 0, 2, GL_FLOAT, 7 * sizeof(float), (void*)0);
    particleBatch.batchVAO->LinkAttrib(*particleBatch.batchVBO, 1, 3, GL_FLOAT, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    particleBatch.batchVAO->LinkAttrib(*particleBatch.batchVBO, 2, 2, GL_FLOAT, 7 * sizeof(float), (void*)(5 * sizeof(float)));

    particleBatch.batchVAO->Unbind();
}

void OpenGLRendererWrapper::BeginBatch() {
    if (!shader2D) return;

    particleBatch.vertices.clear();
    particleBatch.indices.clear();
    particleBatch.particleCount = 0;
}

void OpenGLRendererWrapper::AddCircleToBatch(const glm::vec2& position, float radius, const glm::vec3& color) {
    if (!shader2D) return;

    // Add vertices for a circle quad
    unsigned int baseIndex = particleBatch.particleCount * 4;

    // Bottom-left
    particleBatch.vertices.insert(particleBatch.vertices.end(), {
        position.x - radius, position.y - radius,  // Position
        color.r, color.g, color.b,                 // Color
        -1.0f, -1.0f                               // TexCoord for circle
    });

    // Bottom-right
    particleBatch.vertices.insert(particleBatch.vertices.end(), {
        position.x + radius, position.y - radius,
        color.r, color.g, color.b,
        1.0f, -1.0f
    });

    // Top-right
    particleBatch.vertices.insert(particleBatch.vertices.end(), {
        position.x + radius, position.y + radius,
        color.r, color.g, color.b,
        1.0f, 1.0f
    });

    // Top-left
    particleBatch.vertices.insert(particleBatch.vertices.end(), {
        position.x - radius, position.y + radius,
        color.r, color.g, color.b,
        -1.0f, 1.0f
    });

    // Add indices for the quad
    particleBatch.indices.insert(particleBatch.indices.end(), {
        baseIndex + 0, baseIndex + 1, baseIndex + 2,
        baseIndex + 2, baseIndex + 3, baseIndex + 0
    });

    particleBatch.particleCount++;
}

void OpenGLRendererWrapper::RenderBatch() {
    if (!shader2D || !particleBatch.batchVAO || particleBatch.particleCount == 0) return;

    // Update buffer data
    particleBatch.batchVBO->Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    particleBatch.vertices.size() * sizeof(float),
                    particleBatch.vertices.data());

    particleBatch.batchEBO->Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    particleBatch.indices.size() * sizeof(unsigned int),
                    particleBatch.indices.data());

    // Set uniforms for circle batch rendering
    GLuint modelLoc = glGetUniformLocation(shader2D->ID, "model2D");
    glm::mat3 identityMatrix = glm::mat3(1.0f);
    glUniformMatrix3fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identityMatrix));

    GLuint shapeLoc = glGetUniformLocation(shader2D->ID, "shapeType");
    glUniform1i(shapeLoc, 1); // 1 = circle

    GLuint smoothLoc = glGetUniformLocation(shader2D->ID, "smoothness");
    glUniform1f(smoothLoc, 0.02f);

    // Render the batch
    particleBatch.batchVAO->Bind();
    glDrawElements(GL_TRIANGLES, particleBatch.indices.size(), GL_UNSIGNED_INT, 0);
    particleBatch.batchVAO->Unbind();
}

void OpenGLRendererWrapper::EndBatch() {
    // Nothing special needed here for now
}

GLuint OpenGLRendererWrapper::GetViewportTexture() const {
    return viewportFramebuffer ? viewportFramebuffer->GetTexture() : 0;
}

void OpenGLRendererWrapper::ResizeViewport(int width, int height) {
    if (viewportFramebuffer && (width > 0 && height > 0)) {
        viewportFramebuffer->Resize(width, height);
        viewportWidth = width;
        viewportHeight = height;

        // Update camera aspect ratio
        if (camera) {
            camera->width = width;
            camera->height = height;
        }
    }
}

void OpenGLRendererWrapper::SetMainWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    // Update 2D camera viewport as well
    if (camera2D) {
        camera2D->SetViewportSize(width, height);
}
}
} // namespace Common
} // namespace Engine
