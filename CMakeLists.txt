cmake_minimum_required(VERSION 3.15)
project(visualizer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_definitions(NOMINMAX)
endif()

# Find Conan packages
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(miniaudio REQUIRED)
find_package(kissfft REQUIRED)

# Setup executable
add_executable(visualizer src/main.cpp src/Window.cpp src/AudioEngine.cpp src/Shader.cpp)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Link Conan targets
target_link_libraries(visualizer PRIVATE
    glfw
    glad::glad
    glm::glm
    miniaudio::miniaudio
    kissfft::kissfft
)
