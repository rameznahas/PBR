#pragma once
#include <random>
#include "glfw3.h"
#include "Window.h"
#include "Camera.h"
#include "Light.h"
#include "Shader.h"
#include "Model.h"

//#define FULLSCREEN
//#define SCREEN_GRAB

#define CAM_SPEED 2.0f
#define NB_CUBEMAP_FACES 6
#define NB_SPHERE_ROWS 7
#define NB_SPHERE_COLS 7
#define NB_HDR_TEX 2
#define NB_SCENES 6
#define NB_TEX_SPHERE 5
#define ENV_MAP_RES 4096
#define IRR_MAP_RES 128
#define PRE_FILTERED_ENV_MAP_RES 512
#define BRDF_INT_MAP_RES 1024
#define TEX_PER_MAT 5
#define WORLD_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define WORLD_FWD glm::vec3(0.0f, 0.0f, 1.0f)

bool init();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint loadCubemap(std::vector<std::string>& paths);
GLuint loadTexture(const char* path, bool gammaCorrection);
GLuint loadHDRmap(const char* path);
void computeVertTangents(float* vertices, float* to);
void initVertexAttributes(GLuint& VAO, GLuint& VBO, GLfloat* data, GLuint size, GLuint nb_attrib, GLuint stride, GLuint* attribSizes);

Window window;

Camera camera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -3.0f),
	WORLD_UP,
	45.0f
);

Point_Light pointLight(
	glm::vec3(0.5f, 1.0f, 0.3f),
	glm::vec3(1.0f),
	glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)
);

glm::mat4 M, V, P, MVP;
glm::mat3 NM;

const char* texPaths[3] = {
	"./assets/textures/parallax/wood.png",
	"./assets/textures/parallax/toy_box_normal.png",
	"./assets/textures/parallax/toy_box_disp.png"
};

float heightScale = 0.1f;
bool parallax = false;

double delta_time = 0.0;
double last_frame = 0.0f;
double current_frame;
float angle_x = 0.0f;
float angle_y = 0.0f;

GLfloat quadVertices[] = {
	// positions			// normals				// tangents				// uvs
	-1.0f, -1.0f,  0.0f,	0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
	 1.0f, -1.0f,  0.0f,	0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
	 1.0f,  1.0f,  0.0f,	0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f,  0.0f,	0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
	-1.0f,  1.0f,  0.0f,	0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f,	0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f
};