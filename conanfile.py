from conan import ConanFile
from conan.tools.cmake import cmake_layout

class VisualizerConan(ConanFile):
    name = "visualizer"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("glad/0.1.36")
        self.requires("glm/0.9.9.8")
        self.requires("miniaudio/0.11.21")
        self.requires("kissfft/131.1.0")
        self.requires("imgui/1.91.8")
        self.requires("portable-file-dialogs/0.1.0")

    default_options = {"imgui/*:shared": False}

    def build_requirements(self):
        self.test_requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)
