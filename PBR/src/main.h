#pragma once
#include "glfw3.h"
#include "Window.h"
#include "Camera.h"
#include "Light.h"
#include "Shader.h"
#include "Model.h"

#define NB_POINT_LIGHTS 4
#define NB_CUBEMAP_FACES 6
#define CUBEMAP_RESOLUTION 2048
#define WORLD_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define WORLD_FWD glm::vec3(0.0f, 0.0f, 1.0f)

bool init();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void poll_mouse(Model& model);
GLuint load_cubemap(std::vector<std::string>& paths);
void modelsBufferSubData();

Window window;

Camera camera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -3.0f),
	WORLD_UP,
	45.0f
);

Directional_Light directional_light(
	glm::vec3(100.0f, -100.0f, 100.0f),
	glm::vec3(1.0f),
	glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f)
);

glm::mat4 M, V, P, MVP;
glm::mat3 normal_matrix;

const unsigned int PADDED_VEC3 = sizeof(glm::vec3) + 4;
const unsigned int nm_base_offset = 2 * sizeof(glm::mat4);

double delta_time = 0.0;
bool mouse_button_down = false;
float angle_x = 0.0f;
float angle_y = 0.0f;

GLfloat cubeVertices[] = {
	// positions				// normals			// uvs
	// back face
	-1.0f, -1.0f, -1.0f,		0.0f,  0.0f,		-1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f,  1.0f, -1.0f,		0.0f,  0.0f,		-1.0f, 1.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,		0.0f,  0.0f,		-1.0f, 1.0f, 0.0f, // bottom-right         
	 1.0f,  1.0f, -1.0f,		0.0f,  0.0f,		-1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,		0.0f,  0.0f,		-1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,		0.0f,  0.0f,		-1.0f, 0.0f, 1.0f, // top-left
	// front face				
	-1.0f, -1.0f,  1.0f,		0.0f,  0.0f,		 1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,		0.0f,  0.0f,		 1.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,		0.0f,  0.0f,		 1.0f, 1.0f, 1.0f, // top-right
	 1.0f,  1.0f,  1.0f,		0.0f,  0.0f,		 1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,		0.0f,  0.0f,		 1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,		0.0f,  0.0f,		 1.0f, 0.0f, 0.0f, // bottom-left
	// left face
	-1.0f,  1.0f,  1.0f,	-	1.0f,  0.0f,		 0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f,	-	1.0f,  0.0f,		 0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f,	-	1.0f,  0.0f,		 0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f,	-	1.0f,  0.0f,		 0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,	-	1.0f,  0.0f,		 0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f,	-	1.0f,  0.0f,		 0.0f, 1.0f, 0.0f, // top-right
	// right face
	 1.0f,  1.0f,  1.0f,		1.0f,  0.0f,		 0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f, -1.0f,		1.0f,  0.0f,		 0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,		1.0f,  0.0f,		 0.0f, 1.0f, 1.0f, // top-right         
	 1.0f, -1.0f, -1.0f,		1.0f,  0.0f,		 0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,		1.0f,  0.0f,		 0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f,  1.0f,		1.0f,  0.0f,		 0.0f, 0.0f, 0.0f, // bottom-left     
	// bottom face				
	-1.0f, -1.0f, -1.0f,		0.0f, -1.0f,		 0.0f, 0.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,		0.0f, -1.0f,		 0.0f, 1.0f, 1.0f, // top-left
	 1.0f, -1.0f,  1.0f,		0.0f, -1.0f,		 0.0f, 1.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,		0.0f, -1.0f,		 0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,		0.0f, -1.0f,		 0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,		0.0f, -1.0f,		 0.0f, 0.0f, 1.0f, // top-right
	// top face					
	-1.0f,  1.0f, -1.0f,		0.0f,  1.0f,		 0.0f, 0.0f, 1.0f, // top-left
	 1.0f,  1.0f , 1.0f,		0.0f,  1.0f,		 0.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,		0.0f,  1.0f,		 0.0f, 1.0f, 1.0f, // top-right     
	 1.0f,  1.0f,  1.0f,		0.0f,  1.0f,		 0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,		0.0f,  1.0f,		 0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,		0.0f,  1.0f,		 0.0f, 0.0f, 0.0f  // bottom-left
};

GLfloat planeVertices[] = {
	// positions				// normals				// texcoords
     25.0f, -0.5f,  25.0f,		0.0f, 1.0f, 0.0f,		25.0f,  0.0f,
    -25.0f, -0.5f,  25.0f,		0.0f, 1.0f, 0.0f,		 0.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,		0.0f, 1.0f, 0.0f,		 0.0f, 25.0f,
    
     25.0f, -0.5f,  25.0f,		0.0f, 1.0f, 0.0f,		25.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,		0.0f, 1.0f, 0.0f,		 0.0f, 25.0f,
     25.0f, -0.5f, -25.0f,		0.0f, 1.0f, 0.0f,		25.0f, 25.0f
};