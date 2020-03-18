#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static struct Camera_Movement
{
    bool
        forward = false,
        back = false,
        left = false,
        right = false;
} CamMove;

constexpr float MAXLOOKANGLE = 60.0f;


class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    bool onGround;
    bool isFalling;


    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        onGround = false;
        isFalling = false;

        MovementSpeed = 4.0f;
        MouseSensitivity = 0.5f;

        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(float deltaTime)
    {
        // Handle keyboard movement
        float velocity = MovementSpeed * deltaTime;

        if (CamMove.forward)
            Position += Front * velocity;
        if (CamMove.back)
            Position -= Front * velocity;
        if (CamMove.left)
            Position -= Right * velocity;
        if (CamMove.right)
            Position += Right * velocity;
        updateCameraVectors();
    }

    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        // Handle mouse movement
        Yaw += (xoffset * MouseSensitivity);
        Pitch += (yoffset * MouseSensitivity);

        if (Pitch > MAXLOOKANGLE)
            Pitch = MAXLOOKANGLE;
        if (Pitch < -MAXLOOKANGLE)
            Pitch = -MAXLOOKANGLE;
        updateCameraVectors();
        std::cout << "Front: " << Front.x << "," << Front.y << "," << Front.z << "\n";
    }
    void Update(int deltaTime)
    {

        // Any boundary checking or collision detection could go here

    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // reset Right and Up
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));

    }

};