#include "../../Common/include/RendererWrapper.h"
#include <iostream>

using namespace Engine;

int main() {
    // Create and initialize the renderer
    Common::OpenGLRendererWrapper renderer;
    if (!renderer.Initialize(1300, 900, "PlaneEngine")) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }

    // Main loop
    while (!glfwWindowShouldClose(renderer.GetWindow())) {
        renderer.BeginFrame();

        // For now, just a clear screen to test if things are working

        renderer.EndFrame();
    }

    renderer.Shutdown();
    return 0;
}
