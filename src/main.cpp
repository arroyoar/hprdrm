#include <iostream>
#include "Window.h"

int main() {
    std::cout << "Starting 3D Music Visualizer..." << std::endl;

    Window window(1280, 720, "3D Music Visualizer");

    if (!window.initialize()) {
        std::cerr << "Failed to initialize application window." << std::endl;
        return -1;
    }

    // Main application loop
    while (!window.shouldClose()) {
        // Process Input (Placeholder)
        if (glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window.getHandle(), true);
        }

        // Render
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f); // Dark gray background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Swap buffers and poll IO events
        window.swapBuffers();
        window.pollEvents();
    }

    std::cout << "Exiting 3D Music Visualizer." << std::endl;
    return 0;
}
