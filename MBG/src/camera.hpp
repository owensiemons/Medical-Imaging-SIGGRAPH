#pragma once

#include "../MBG/OpenGL/Window.hpp"
#include <glm/glm.hpp>

class Camera {
public:
    Camera(glm::vec3 position, float aspectRatio);
    ~Camera();

    glm::mat4 getCameraViewMat();
    glm::mat4 getCameraProjMat();

    void rotate(glm::vec2 rot);
    void updateAspectRatio(float aspectRatio);
    float getFOV();
    void updateFOV(float fov);
    void orbit(glm::vec2 mouseAngles, glm::vec3 target, float rad);
    glm::vec3 getDirection();

    glm::vec3 position;
    const float near_plane = 0.1f;
    const float far_plane = 100000.0f;

private:
    glm::vec3 direction = glm::vec3(0.0, 0.0, 1.0);
    glm::vec2 rotation = glm::vec2(0.0);
    glm::mat4 perspective;
    float aspectRatio_;
    float zoom = 70.0f;
};