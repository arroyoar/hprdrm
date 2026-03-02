#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

enum Camera_Mode {
    MODE_MANUAL,
    MODE_ORBIT,
    MODE_SWEEP
};

const float YAW         = -90.0f;
const float PITCH       =  -20.0f;
const float SPEED       =  20.0f; // Increased default speed for the larger grid
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    Camera_Mode Mode;
    float OrbitRadius;
    float OrbitSpeed;

    Camera(glm::vec3 position = glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Mode(MODE_MANUAL), OrbitRadius(100.0f), OrbitSpeed(0.2f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void Update(float time, float deltaTime) {
        if (Mode == MODE_ORBIT) {
            // Circular orbit around the origin
            float camX = sin(time * OrbitSpeed) * OrbitRadius;
            float camZ = cos(time * OrbitSpeed) * OrbitRadius;
            Position = glm::vec3(camX, 60.0f, camZ);
            
            // Look at the center
            glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
            Front = glm::normalize(center - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));  
            Up    = glm::normalize(glm::cross(Right, Front));
            
            // Re-calculate Yaw/Pitch based on the new Front vector so switching back to manual is smooth
            Pitch = glm::degrees(asin(Front.y));
            Yaw = glm::degrees(atan2(Front.z, Front.x));
            
        } else if (Mode == MODE_SWEEP) {
            // Figure-8 sweeping movement
            float camX = sin(time * OrbitSpeed) * OrbitRadius;
            float camZ = sin(time * OrbitSpeed * 0.5f) * OrbitRadius;
            float camY = 30.0f + sin(time * OrbitSpeed * 2.0f) * 20.0f; // Bobs up and down
            Position = glm::vec3(camX, camY, camZ);
            
            // Look slightly ahead of the origin
            glm::vec3 lookTarget = glm::vec3(0.0f, -10.0f, 0.0f);
            Front = glm::normalize(lookTarget - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));  
            Up    = glm::normalize(glm::cross(Right, Front));
            
            Pitch = glm::degrees(asin(Front.y));
            Yaw = glm::degrees(atan2(Front.z, Front.x));
        }
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        if (Mode != MODE_MANUAL) return; // Disable WASD in cinematic modes
        
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        if (Mode != MODE_MANUAL) return; // Disable mouse look in cinematic modes
        
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));  
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
