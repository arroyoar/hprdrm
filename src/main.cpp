#include <iostream>
#include <vector>
#include "Window.h"
#include "AudioEngine.h"
#include "Shader.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

// Camera setup
Camera camera(glm::vec3(0.0f, 15.0f, 30.0f));
float lastX = 1280.0f / 2.0;
float lastY = 720.0f / 2.0;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    // Only move camera if right mouse button is pressed to avoid locking mouse constantly
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.ProcessMouseMovement(xoffset, yoffset);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // Reset first mouse to prevent jumping when right clicking again
        firstMouse = true; 
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Zoom -= (float)yoffset;
    if (camera.Zoom < 1.0f) camera.Zoom = 1.0f;
    if (camera.Zoom > 45.0f) camera.Zoom = 45.0f;
}

int main() {
    std::cout << "Starting 3D Music Visualizer..." << std::endl;

    Window window(1280, 720, "3D Music Visualizer");
    if (!window.initialize()) {
        std::cerr << "Failed to initialize application window." << std::endl;
        return -1;
    }

    glfwSetCursorPosCallback(window.getHandle(), mouse_callback);
    glfwSetScrollCallback(window.getHandle(), scroll_callback);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    Shader shader("assets/shaders/visualizer.vert", "assets/shaders/visualizer.frag");

    AudioEngine audio;
    if (audio.initialize("music.mp3")) {
        audio.play();
    } else {
        std::cerr << "Warning: Could not load music.mp3. Visualizer will run without audio." << std::endl;
    }

    // Cube vertices (36 vertices for 12 triangles)
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    // Generate grid offsets
    std::vector<glm::vec3> translations;
    int gridSize = 50; // 50x50 = 2500 cubes
    float offset = 1.2f;
    for (int z = -gridSize / 2; z < gridSize / 2; z++) {
        for (int x = -gridSize / 2; x < gridSize / 2; x++) {
            translations.push_back(glm::vec3(x * offset, 0.0f, z * offset));
        }
    }

    // Instance VBO
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3), &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Cube VAO/VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // Instance attribute (offset)
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1); // Advance once per instance

    // Main application loop
    while (!window.shouldClose()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window.getHandle());
        audio.update();

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setFloat("uBass", audio.getBass());
        shader.setFloat("uMid", audio.getMid());
        shader.setFloat("uTreble", audio.getTreble());

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window.getWidth() / (float)window.getHeight(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glBindVertexArray(VAO);
        // Draw 2500 cubes with one call
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, translations.size());
        glBindVertexArray(0);

        window.swapBuffers();
        window.pollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);

    return 0;
}