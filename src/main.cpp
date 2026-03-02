#include <iostream>
#include <vector>
#include <filesystem>
#include "Window.h"
#include "AudioEngine.h"
#include "Shader.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

// Camera setup - Pull the camera back and up to see the full 50x50 grid
Camera camera(glm::vec3(0.0f, 60.0f, 90.0f));
float lastX = 1280.0f / 2.0;
float lastY = 720.0f / 2.0;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window, AudioEngine& audio) {
// ... omitting existing processInput content to match the old string start

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

    // Audio controls (with simple debouncing)
    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!spacePressed) {
            audio.togglePause();
            spacePressed = true;
        }
    } else {
        spacePressed = false;
    }

    static bool nPressed = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!nPressed) {
            audio.nextTrack();
            nPressed = true;
        }
    } else {
        nPressed = false;
    }

    static bool pPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!pPressed) {
            audio.prevTrack();
            pPressed = true;
        }
    } else {
        pPressed = false;
    }

    static bool rightPressed = false;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!rightPressed) {
            audio.seekForward(5.0f); // Seek forward 5 seconds
            rightPressed = true;
        }
    } else {
        rightPressed = false;
    }

    // Camera Mode toggle (C key)
    static bool cPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cPressed) {
            if (camera.Mode == MODE_MANUAL) {
                camera.Mode = MODE_ORBIT;
                std::cout << "Camera Mode: ORBIT" << std::endl;
            } else if (camera.Mode == MODE_ORBIT) {
                camera.Mode = MODE_SWEEP;
                std::cout << "Camera Mode: SWEEP" << std::endl;
            } else {
                camera.Mode = MODE_MANUAL;
                std::cout << "Camera Mode: MANUAL" << std::endl;
            }
            cPressed = true;
        }
    } else {
        cPressed = false;
    }
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

int main(int argc, char* argv[]) {
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
    bool audioLoaded = false;
    
    // 1. Check if user provided a custom directory or file path via command line
    if (argc > 1) {
        std::string customPath = argv[1];
        std::cout << "Attempting to load music from: " << customPath << std::endl;
        
        if (std::filesystem::exists(customPath)) {
            if (std::filesystem::is_directory(customPath)) {
                audioLoaded = audio.loadDirectory(customPath);
            } else {
                audioLoaded = audio.initialize(customPath);
            }
        }
        
        if (!audioLoaded) {
            std::cerr << "Failed to load audio from custom path: " << customPath << ". Falling back to defaults." << std::endl;
        }
    }

    // 2. Fallback defaults if no custom path was provided or it failed
    if (!audioLoaded) {
        // Try to load a directory called "music"
        audioLoaded = audio.loadDirectory("music");
        if (!audioLoaded) audioLoaded = audio.loadDirectory("../music");
        if (!audioLoaded) audioLoaded = audio.loadDirectory("../../music");
        
        // Fallback to the single music.mp3 file
        if (!audioLoaded) audioLoaded = audio.initialize("music.mp3");
        if (!audioLoaded) audioLoaded = audio.initialize("../../music.mp3");
        if (!audioLoaded) audioLoaded = audio.initialize("../music.mp3");
    }
    
    if (audioLoaded) {
        audio.play();
    } else {
        std::cerr << "Warning: Could not load any music. Visualizer will run without audio." << std::endl;
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
    int gridSize = 100; // Increased to 100x100 = 10,000 cubes for a much wider field
    float offset = 2.5f; // Increased spacing between cubes
    for (int z = -gridSize / 2; z < gridSize / 2; z++) {
        for (int x = -gridSize / 2; x < gridSize / 2; x++) {
            // Stagger every other row on the X axis so we can see "through" the grid better
            float rowOffset = (z % 2 == 0) ? 0.0f : (offset / 2.0f);
            
            float xPos = x * offset + rowOffset;
            float zPos = z * (offset * 0.866f); // Pull rows closer for hexagonal pattern
            
            // Add a slight curve-down effect so outer layers sit lower, exposing the center
            float distFromCenter = std::sqrt(xPos*xPos + zPos*zPos);
            float yPos = distFromCenter * -0.1f; 

            translations.push_back(glm::vec3(xPos, yPos, zPos));
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

    std::string currentTrackName = "";

    // Main application loop
    while (!window.shouldClose()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window.getHandle(), audio);
        camera.Update(currentFrame, deltaTime);
        audio.update();

        // Update Window Title with track info
        std::string trackName = audio.getCurrentTrackName();
        if (trackName != currentTrackName) {
            currentTrackName = trackName;
            std::string title = "3D Music Visualizer - " + currentTrackName;
            if (!audio.isPlaying() && audioLoaded) title += " (Paused)";
            glfwSetWindowTitle(window.getHandle(), title.c_str());
        } else if (!audio.isPlaying() && audioLoaded) {
            std::string title = "3D Music Visualizer - " + currentTrackName + " (Paused)";
            glfwSetWindowTitle(window.getHandle(), title.c_str());
        }

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setFloat("uSubBass", audio.getSubBass());
        shader.setFloat("uBass", audio.getBass());
        shader.setFloat("uLowMid", audio.getLowMid());
        shader.setFloat("uMid", audio.getMid());
        shader.setFloat("uHighMid", audio.getHighMid());
        shader.setFloat("uPresence", audio.getPresence());
        shader.setFloat("uTreble", audio.getTreble());

        shader.setFloat("uSubBassMaxDur", audio.getSubBassMaxDuration());
        shader.setFloat("uBassMaxDur", audio.getBassMaxDuration());
        shader.setFloat("uLowMidMaxDur", audio.getLowMidMaxDuration());
        shader.setFloat("uMidMaxDur", audio.getMidMaxDuration());
        shader.setFloat("uHighMidMaxDur", audio.getHighMidMaxDuration());
        shader.setFloat("uPresenceMaxDur", audio.getPresenceMaxDuration());
        shader.setFloat("uTrebleMaxDur", audio.getTrebleMaxDuration());

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window.getWidth() / (float)window.getHeight(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glBindVertexArray(VAO);
        // Draw 10000 cubes with one call
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