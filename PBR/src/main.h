#pragma once
#include "glfw3.h"
#include "Window.h"
#include "Camera.h"
#include "Light.h"

#define NB_POINT_LIGHTS 4
#define NB_CUBEMAP_FACES 6
#define WORLD_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define WORLD_FWD glm::vec3(0.0f, 0.0f, 1.0f)

bool init();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
GLuint load_cubemap(std::vector<std::string>& paths);

Window window;

glm::vec3 backpack_mirror_pos(0.0f, 0.0f, -10.0f);

Camera camera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -3.0f),
	WORLD_UP,
	45.0f
);

Camera environment_mapping_cameras[NB_CUBEMAP_FACES] = {
	Camera(backpack_mirror_pos,  WORLD_RIGHT, WORLD_UP, 45.0f),
	Camera(backpack_mirror_pos, -WORLD_RIGHT, WORLD_UP, 45.0f),
	Camera(backpack_mirror_pos,  WORLD_UP,	  WORLD_FWD, 45.0f),
	Camera(backpack_mirror_pos, -WORLD_UP,   -WORLD_FWD, 45.0f),
	Camera(backpack_mirror_pos, -WORLD_FWD,   WORLD_UP, 45.0f),
	Camera(backpack_mirror_pos,  WORLD_FWD,   WORLD_UP, 45.0f),
};

Directional_Light directional_light(
	glm::vec3(-0.2f, -1.0f, -0.3f),
	glm::vec3(1.0f),
	glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f)
);

Point_Light point_lights[NB_POINT_LIGHTS] = {
	Point_Light(
		glm::vec3(0.7f, 0.2f, 2.0f),
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(-4.0f, 2.0f, -12.0f),
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(0.0f, 0.0f, -3.0f),
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	)
};

Spotlight spotlight(
	glm::vec3(0.0f, 2.0f, 5.0f),
	glm::vec3(0.0f, -2.0f, -5.0f),
	glm::vec3(1.0f),
	glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f),
	1.0f, 0.09f, 0.032f,
	12.5f, 15.0f
);

glm::mat4 V, P, MVP;
glm::mat3 normal_matrix;

double delta_time = 0.0;
bool mouse_button_down = false;
float angle_x = 0.0f;
float angle_y = 0.0f;

GLfloat light_vertices[] = {
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f
};

GLuint light_indices[] = {
	0, 1, 2,
	0, 2, 3,
	4, 6, 5,
	4, 7, 6,
	1, 5, 2,
	5, 6, 2,
	0, 3, 4,
	3, 7, 4
};

GLfloat skybox_vertices[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};