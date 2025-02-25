// Eyub Celebioglu
#ifndef CAMERA_H
#define CAMERA_H

#include <glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // vect de camera
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler
    float Yaw;
    float Pitch;

    // option de camera
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructeur
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f))
        : Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
        , Yaw(-90.0f)
        , Pitch(0.0f)
        , MovementSpeed(2.5f)
        , MouseSensitivity(0.1f)
        , Zoom(45.0f)
    {
        Position = position;
        updateCameraVectors();
    }

    // retourne la nmatrice de vue
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // mouv clavier
    void ProcessKeyboard(int direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 0) // dev
            Position += Front * velocity;
        if (direction == 1) // der
            Position -= Front * velocity;
        if (direction == 2) // gauche
            Position -= Right * velocity;
            // std::cout << "touche" ; 
        if (direction == 3) // droite
            Position += Right * velocity;
    }

    // traite le mouv souris
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
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

    // traite le zoom 
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    void updateCameraVectors() {
        // calcule le nouveau vecteur front
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        
        // recalcule les vecteurs right et up
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif