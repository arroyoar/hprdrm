# 3D Music Visualizer Project Plan

This project is a 3D music visualizer written in C++, using OpenGL, GLFW, GLM, miniaudio, and KissFFT. The build is managed by CMake and Conan 2.x, with tasks automated via Python's `invoke`.

## Phase 1: Environment Setup & Tooling
*   **Python Virtual Environment:** Create a `requirements.txt` containing `invoke` and `conan`.
*   **Task Automation (`tasks.py`):**
    *   `inv bootstrap`: Sets up the Conan profile, installs C++ dependencies, and generates CMake toolchain files.
    *   `inv build`: Runs CMake configure and build steps.
    *   `inv clean`: Cleans the build and generator directories.
    *   `inv run`: Builds and executes the visualizer.
*   **Conan Configuration (`conanfile.py`):** Define dependencies: `glfw`, `glad`, `glm`, `miniaudio`, and `kissfft`.
*   **CMake Setup (`CMakeLists.txt`):** Configure the project to consume Conan-generated targets and build the C++ executable.

## Phase 2: Core Architecture & Initialization
*   **Window Management:** Initialize GLFW, create a window, and set up the OpenGL context.
*   **Main Loop:** Implement the core application loop (process input, update state, render frame, swap buffers).
*   **Basic Shader System:** Write a basic vertex and fragment shader to render 3D geometry with basic RGB colors.

## Phase 3: Audio Processing Engine
*   **Playback System:** Initialize `miniaudio` to load and play an audio file (e.g., MP3 or WAV).
*   **Audio Callback & FFT:** Capture the audio stream data as it plays. Feed this data into `KissFFT` to perform a Fast Fourier Transform.
*   **Frequency Analysis:** Group the resulting frequency bins into distinct frequency bands (e.g., Bass, Mid, Treble) and calculate their magnitudes. This data will be exposed to the renderer.

## Phase 4: 3D Rendering & Camera System
*   **3D Camera:** Implement a "Fly Camera" or "Orbit Camera" using GLM. Bind mouse and keyboard inputs via GLFW to move the camera freely through 3D space.
*   **Grid System:** Render a 3D grid of basic geometric shapes (cubes or spheres). We will use instanced rendering in OpenGL to ensure high performance when drawing thousands of blocks.

## Phase 5: Music Synchronization (Basic Visuals)
*   **Data Binding:** Map the frequency band data (from Phase 3) to the properties of the 3D grid (from Phase 4).
*   **Visual Reactivity:** 
    *   Make blocks scale on the Y-axis (height) based on frequency amplitude.
    *   Interpolate block RGB colors based on the intensity of the music.
*   **Smoothing:** Implement time-based smoothing (interpolation/lerping) so the visualizer movements are fluid rather than jittery.

## Phase 6: Features & Polish
*   **Audio Controls:** Add play, pause, fast forward, skip song, and display the current song name.
*   **Playlist Support:** Add the ability to read and play from a directory of music files.
*   **Cinematic Camera:** Add pre-determined automated camera movements (sweeps, tracking, orbiting).
*   **Visual Refinement:** Modify the max-duration flash tracking so the inner sub-bass/bass frequencies aren't constantly flashing white during heavy bass sections.

## Phase 7: Advanced Architecture (Preparation for Future Features)
*   To support advanced features (Depth of Field, Bloom/Sparks, Distortion, Video Texturing), design the renderer with a **Framebuffer Object (FBO)** architecture from the start.
*   Render the 3D grid to a texture instead of directly to the screen.
*   Ensure the geometric rendering system can accept texture coordinates (UVs) for future image/video mapping.
