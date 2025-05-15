#include "../../Common/include/RendererWrapper.h"
#include <iostream>

int main() {
    // Create and initialize renderer
    Engine::Common::OpenGLRendererWrapper renderer;

    if (!renderer.Initialize(1300, 900, "PlaneEngine")) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }

    // Main loop
    while (!glfwWindowShouldClose(renderer.GetWindow())) {
        renderer.BeginFrame();
        // The rendering is now handled inside BeginFrame
        renderer.EndFrame();
    }

    renderer.Shutdown();
    return 0;
}
