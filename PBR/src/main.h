#pragma once
#include "glfw3.h"
#include "Window.h"
#include "Camera.h"
#include "Light.h"
#include "Shader.h"
#include "Model.h"

#define CAM_SPEED 3.0f
#define NB_POINT_LIGHTS 4
#define NB_CUBEMAP_FACES 6
#define NB_MODELS 4
#define NB_CUBES 3
#define SHADOWMAP_RESOLUTION 2048
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
void modelsBufferSubData(int idx);
void renderShadowScene(GLuint VAOcube, GLuint VAOplane, GLuint UBOshadowMap);
void renderScene(GLuint VAOcube, GLuint VAOplane, GLuint UBOlighting, GLuint UBOmodels, unsigned int offset);

Window window;

Camera camera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -3.0f),
	WORLD_UP,
	45.0f
);

Camera shadowCam(
	glm::vec3(-2.0f, 4.0f, -1.0f),
	glm::vec3(2.0f, -4.0f, 1.0f),
	WORLD_UP,
	45.0f
);

Point_Light pointLight(
	glm::vec3(-2.0f, 4.0f, -1.0f),
	glm::vec3(1.0f),
	glm::vec3(0.15f), glm::vec3(0.5f), glm::vec3(1.0f)
);

glm::mat4 M[NB_MODELS];
glm::mat4 MVP[NB_MODELS];
glm::mat4 V, P;
glm::mat3 normal_matrix[NB_MODELS];
glm::mat4 lightSpaceMatrix;

const unsigned int nm_base_offset = 3 * sizeof(glm::mat4);

double delta_time = 0.0;
bool mouse_button_down = false;
float angle_x = 0.0f;
float angle_y = 0.0f;
int samples = 3;

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