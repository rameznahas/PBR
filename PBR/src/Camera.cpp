#include "Camera.h"
#include <iostream>

Camera::Camera(glm::vec3 pos, glm::vec3 fwd, glm::vec3 world_up, float fov) 
	:
	position(pos),
	world_up(world_up),
	fov(fov),
	first_time(true)
{
	update_axes(fwd);
	pitch = glm::degrees(asin(forward.y));
	yaw = glm::degrees(atan2(forward.z, forward.x));

}

void Camera::walk_around(glm::vec3 dir, double delta_time) {
	position += dir * (float)delta_time * 5.0f;
}

void Camera::look_around(double xpos, double ypos) {
	static double prev_x, prev_y;

	if (first_time) {
		prev_x = xpos;
		prev_y = ypos;
		first_time = false;
	}

	double x_offset = xpos - prev_x;
	double y_offset = prev_y - ypos;
	prev_x = xpos;
	prev_y = ypos;

	float sensitivity = 0.05f;
	x_offset *= sensitivity;
	y_offset *= sensitivity;

	yaw += (float)x_offset;
	pitch += (float)y_offset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	update_axes(glm::normalize(glm::vec3(
		cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
		sin(glm::radians(pitch)),
		cos(glm::radians(pitch)) * sin(glm::radians(yaw))
	)));
}

void Camera::zoom(double yoffset) {
	fov -= (float)yoffset;

	if (fov < 1.0f) fov = 1.0f;
	if (fov > 80.0f) fov = 80.0f;
}

glm::mat4 Camera::get_view_matrix() {
	return glm::lookAt(position, position + forward, up);
}

void Camera::update_axes(const glm::vec3 fwd) {
	forward = glm::normalize(fwd);
	right = glm::normalize(glm::cross(forward, world_up));
	up = glm::normalize(glm::cross(right, forward));
}