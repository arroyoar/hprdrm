#include <iostream>
#include <vector>
#include <filesystem>
#include "Window.h"
#include "AudioEngine.h"
#include "Shader.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

// ImGui and Portable File Dialogs
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <portable-file-dialogs.h>

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
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse) {
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

    // Setup ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window.getHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Shader shader("assets/shaders/visualizer.vert", "assets/shaders/visualizer.frag");
    Shader postProcessShader("assets/shaders/postprocess.vert", "assets/shaders/postprocess.frag");

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

    // Generate 3D Voxel Grid offsets
    std::vector<glm::vec3> translations;
    int gridSize = 100; // 100x100 = 10,000 horizontal positions
    int stackHeight = 36; // We create 36 vertical layers per position -> 360,000 cubes!
    float offset = 2.5f; // Horizontal spacing between stacks
    
    translations.reserve(gridSize * gridSize * stackHeight);

    for (int y = 0; y < stackHeight; y++) {
        for (int z = -gridSize / 2; z < gridSize / 2; z++) {
            for (int x = -gridSize / 2; x < gridSize / 2; x++) {
                // Stagger every other row on the X axis so we can see "through" the grid better
                float rowOffset = (z % 2 == 0) ? 0.0f : (offset / 2.0f);
                
                float xPos = x * offset + rowOffset;
                float zPos = z * (offset * 0.866f); // Pull rows closer for hexagonal pattern
                
                // We pass the layer index (y) directly into the Y component. 
                // The vertex shader will use this to determine visibility and color!
                translations.push_back(glm::vec3(xPos, (float)y, zPos));
            }
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

    // --- Screen Quad Setup ---
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // --- Framebuffer Object (FBO) Setup ---
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    // Use the window width/height for the texture dimensions
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window.getWidth(), window.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Create a renderbuffer object for depth and stencil attachment
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window.getWidth(), window.getHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    postProcessShader.use();
    postProcessShader.setInt("screenTexture", 0);

    std::string currentTrackName = "";

    // Main application loop
    while (!window.shouldClose()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window.getHandle(), audio);
        camera.Update(currentFrame, deltaTime);
        audio.update();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

        // Render UI Window
        ImGui::Begin("Visualizer Controls");
        
        ImGui::Text("Now Playing: %s", currentTrackName.c_str());
        ImGui::Separator();

        // Playback Controls
        if (ImGui::Button(audio.isPlaying() ? "Pause" : "Play")) {
            audio.togglePause();
        }
        ImGui::SameLine();
        if (ImGui::Button("Prev")) {
            audio.prevTrack();
        }
        ImGui::SameLine();
        if (ImGui::Button("Next")) {
            audio.nextTrack();
        }
        
        ImGui::Separator();
        
        // Playlist Selection
        if (ImGui::Button("Load Music Directory...")) {
            // Open native directory selection dialog
            auto dir = pfd::select_folder("Select Music Directory", ".").result();
            if (!dir.empty()) {
                audioLoaded = audio.loadDirectory(dir);
                if (audioLoaded) {
                    audio.play();
                } else {
                    std::cerr << "No valid audio files found in directory." << std::endl;
                }
            }
        }

        ImGui::Separator();

        // Camera Controls
        ImGui::Text("Camera Mode:");
        if (ImGui::RadioButton("Manual (WASD + Mouse)", camera.Mode == MODE_MANUAL)) { camera.Mode = MODE_MANUAL; }
        if (ImGui::RadioButton("Orbit", camera.Mode == MODE_ORBIT)) { camera.Mode = MODE_ORBIT; }
        if (ImGui::RadioButton("Sweep", camera.Mode == MODE_SWEEP)) { camera.Mode = MODE_SWEEP; }

        ImGui::End();

        // Rendering
        ImGui::Render();

        // 1. FIRST PASS: Render 3D scene to Framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);

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

        // 2. SECOND PASS: Render Screen Quad using the FBO Texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Switch back to default framebuffer
        glDisable(GL_DEPTH_TEST); // Disable depth test so the quad isn't discarded
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        postProcessShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 3. THIRD PASS: Draw UI on top of the Post-Processed Quad
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swapBuffers();
        window.pollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);

    return 0;
}