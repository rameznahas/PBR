#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
	Camera() = default;
	Camera(glm::vec3 pos, glm::vec3 fwd, glm::vec3 world_up, float fov);
	Camera& operator=(const Camera& rhs);
	void walk_around(glm::vec3 dir, double delta_time);
	void look_around(double xpos, double ypos);
	void zoom(double yoffset);
	void moveTo(glm::vec3 pos, glm::vec3 fwd);
	glm::mat4 get_view_matrix();

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	float fov;
	bool first_time;

private:
	void update_axes(const glm::vec3 fwd);

	float pitch;
	float yaw;
	glm::vec3 world_up;
};