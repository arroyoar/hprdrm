#include <gtest/gtest.h>
#include "Camera.h"

// Test that the camera initializes with the correct default values
TEST(CameraTest, DefaultInitialization) {
    Camera camera;
    EXPECT_FLOAT_EQ(camera.Yaw, -90.0f);
    EXPECT_FLOAT_EQ(camera.Pitch, -20.0f);
    EXPECT_FLOAT_EQ(camera.Zoom, 45.0f);
    EXPECT_EQ(camera.Position, glm::vec3(0.0f, 10.0f, 20.0f));
}

// Test that processing keyboard input changes the position correctly
TEST(CameraTest, ProcessKeyboardForward) {
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Front vector defaults to roughly (0, 0, -1) depending on pitch/yaw
    glm::vec3 initialPosition = camera.Position;
    
    // Move forward 1 unit of time
    camera.ProcessKeyboard(FORWARD, 1.0f);
    
    // Expected position should be the front vector multiplied by the movement speed
    glm::vec3 expectedPosition = initialPosition + (camera.Front * camera.MovementSpeed * 1.0f);
    
    EXPECT_FLOAT_EQ(camera.Position.x, expectedPosition.x);
    EXPECT_FLOAT_EQ(camera.Position.y, expectedPosition.y);
    EXPECT_FLOAT_EQ(camera.Position.z, expectedPosition.z);
}

// Test mouse movement constraints
TEST(CameraTest, ProcessMouseMovementConstrainsPitch) {
    Camera camera;
    
    // Try to pitch up by 150 degrees (should be constrained to 89.0f)
    camera.ProcessMouseMovement(0.0f, 150.0f / camera.MouseSensitivity);
    EXPECT_FLOAT_EQ(camera.Pitch, 89.0f);
    
    // Try to pitch down by a huge amount (should be constrained to -89.0f)
    camera.ProcessMouseMovement(0.0f, -200.0f / camera.MouseSensitivity);
    EXPECT_FLOAT_EQ(camera.Pitch, -89.0f);
}
