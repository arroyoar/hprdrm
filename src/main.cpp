#include <iostream>
#include "Window.h"
#include "AudioEngine.h"

int main() {
    std::cout << "Starting 3D Music Visualizer..." << std::endl;

    Window window(1280, 720, "3D Music Visualizer");
    if (!window.initialize()) {
        std::cerr << "Failed to initialize application window." << std::endl;
        return -1;
    }

    AudioEngine audio;
    // We expect a "music.mp3" or "music.wav" in the run directory.
    // If it fails to load, we will still run the visualizer (just without audio reactivity).
    if (audio.initialize("music.mp3")) {
        audio.play();
    } else {
        std::cerr << "Warning: Could not load music.mp3. Visualizer will run without audio." << std::endl;
    }

    // Main application loop
    while (!window.shouldClose()) {
        // Process Input
        if (glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window.getHandle(), true);
        }

        // Update Audio Data
        audio.update();

        // Print frequency bands occasionally to verify (commented out to avoid spam)
        // std::cout << "Bass: " << audio.getBass() << " Mid: " << audio.getMid() << " Treble: " << audio.getTreble() << "\r";

        // Render
        // Change background color based on bass magnitude for a simple visual test
        float bassColor = std::min(audio.getBass() / 100.0f, 1.0f);
        glClearColor(0.1f + bassColor, 0.1f, 0.12f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Swap buffers and poll IO events
        window.swapBuffers();
        window.pollEvents();
    }

    std::cout << "Exiting 3D Music Visualizer." << std::endl;
    return 0;
}
