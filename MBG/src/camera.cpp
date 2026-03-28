#include "camera.hpp"

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 pos, float aspectRatio) {
	position = pos;
	aspectRatio_ = aspectRatio;

	perspective = glm::perspective(glm::radians(zoom), aspectRatio, near_plane, far_plane);
}

glm::mat4 Camera::getCameraViewMat() {
	return glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
}

glm::mat4 Camera::getCameraProjMat() {
	return perspective;
}

void Camera::rotate(glm::vec2 rot) {
	rotation += rot;

	// clamp y rotation so you dont break your neck
	float down_looking_cutoff = 0.0001f;
	rotation.y = glm::clamp(rotation.y, -3.14159265359f / 2.0f + down_looking_cutoff, 3.14159265359f / 2.0f - down_looking_cutoff);

	// loop x so it does not get too big
	if (rotation.x < 0.0) {
		rotation.x += 3.14159265359f * 2.0f;
	}
	if (rotation.x > 3.14159265359f * 2.0f) {
		rotation.x -= 3.14159265359f * 2.0;
	}

	// convert pitch and yaw to a unit vector
	direction.x = glm::cos(rotation.y) * glm::sin(rotation.x);
	direction.y = glm::sin(rotation.y);
	direction.z = glm::cos(rotation.y) * glm::cos(rotation.x);
}

void Camera::updateAspectRatio(float aspectRatio) {
	aspectRatio_ = aspectRatio;
	perspective = glm::perspective(glm::radians(zoom), aspectRatio, near_plane, far_plane);
}

float Camera::getFOV() {
	return zoom;
}
void Camera::updateFOV(float fov) {
	zoom = fov;
	perspective = glm::perspective(glm::radians(zoom), aspectRatio_, near_plane, far_plane);
}

void Camera::orbit(glm::vec2 mouseAngles, glm::vec3 target, float rad) {
	rotate(mouseAngles);
	position = target - direction * rad;
}

glm::vec3 Camera::getDirection() {
	return direction;
}

Camera::~Camera() {
}