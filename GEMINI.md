# Gemini CLI Project Instructions

This file contains specific context and instructions for the Gemini CLI when working in this repository.

## Project Overview
This is a C++ 3D music visualizer project. 

## Technology Stack
*   **Language:** C++ (C++17 or later)
*   **Build System:** CMake
*   **Package Manager:** Conan (v2.x)
*   **Task Automation:** Python with `invoke`
*   **Libraries:** OpenGL, GLFW, GLM, miniaudio, KissFFT

## Development Workflow
1.  All Python dependencies (like `invoke` and `conan`) are managed via `requirements.txt` in a virtual environment.
2.  All C++ dependencies are managed via `conanfile.py`.
3.  All build and run tasks should be executed using `invoke` commands defined in `tasks.py` (e.g., `invoke bootstrap`, `invoke build`, `invoke run`).
4.  Do not invoke CMake or Conan directly unless modifying the build scripts or diagnosing a specific issue; prefer the `invoke` tasks.

## C++ Guidelines
*   Prefer modern C++ paradigms.
*   Ensure memory safety and proper resource management (RAII).
*   Keep the rendering engine decoupled from the audio processing engine as much as possible.
