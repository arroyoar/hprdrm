from invoke import task
import os
import platform

BUILD_DIR = "build"

@task
def clean(c):
    """Cleans the build directory."""
    if os.path.exists(BUILD_DIR):
        if platform.system() == "Windows":
            c.run(f"rmdir /s /q {BUILD_DIR}")
        else:
            c.run(f"rm -rf {BUILD_DIR}")
    print("Cleaned build directory.")

@task
def bootstrap(c, build_type="Release"):
    """Installs C++ dependencies using Conan and generates CMake toolchain."""
    c.run("conan profile detect --force")
    
    # We use build_type to match the CMake configuration
    c.run(f"conan install . --build=missing -s build_type={build_type}")
    print(f"Bootstrap complete for {build_type}.")

@task
def build(c, build_type="Release"):
    """Configures and builds the C++ project using CMake Presets."""
    preset_name = "conan-default" if build_type == "Release" else f"conan-{build_type.lower()}"
    build_preset = "conan-release" if build_type == "Release" else f"conan-{build_type.lower()}"
    
    # Configure CMake
    c.run(f"cmake --preset {preset_name}")
    
    # Build the project
    c.run(f"cmake --build --preset {build_preset}")
    print("Build complete.")

@task
def run(c, build_type="Release"):
    """Runs the compiled visualizer executable."""
    exe_path = os.path.join("build", build_type, "visualizer.exe")
    if platform.system() != "Windows":
        exe_path = os.path.join("build", build_type, "visualizer")
        
    if not os.path.exists(exe_path):
        print(f"Error: Executable not found at {exe_path}. Did you run 'invoke build'?")
        return
        
    c.run(exe_path)

@task
def test(c, build_type="Release"):
    """Runs the C++ unit tests using CTest."""
    build_preset = "conan-release" if build_type == "Release" else f"conan-{build_type.lower()}"
    c.run(f"ctest --preset {build_preset} --output-on-failure")
