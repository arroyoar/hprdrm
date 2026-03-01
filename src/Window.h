#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool initialize();
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLFWwindow* getHandle() const { return m_window; }

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    int m_width;
    int m_height;
    std::string m_title;
};
