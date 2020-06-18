#pragma once
#include <random>
#include "glfw3.h"
#include "Window.h"
#include "Camera.h"
#include "Light.h"
#include "Shader.h"
#include "Model.h"

//#define FULLSCREEN
#define SCREEN_GRAB

#define CAM_SPEED 1.0f
#define NB_POINT_LIGHTS 4
#define NB_CUBEMAP_FACES 6
#define NB_SPHERE_ROWS 7
#define NB_SPHERE_COLS 7
#define NB_HDR_TEX 2
#define NB_SCENES 5
#define HDR_ENV_RES 1024
#define HDR_IRR_RES 32
#define TEX_PER_MAT 3
#define WORLD_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define WORLD_FWD glm::vec3(0.0f, 0.0f, 1.0f)

bool init();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint loadCubemap(std::vector<std::string>& paths);
GLuint loadTexture(const char* path, GLenum tex, bool gammaCorrection);
GLuint loadHDRmap(const char* path);
void computeVertTangents(float* vertices, float* to);
size_t initSphereVertices(GLuint& VAO, GLuint& VBO, GLuint& EBO);
void initVertexAttributes(GLuint& VAO, GLuint& VBO, GLfloat* data, GLuint size, GLuint nb_attrib, GLuint stride, GLuint* attribSizes);
void setUBOtransforms(const glm::mat4& M, const glm::mat4& MVP, const glm::mat3& NM);
void genEnvMap(const char* path, GLuint& texEnvMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms);
void genIrradianceMap(const GLuint& texEnvMap, GLuint& texIrradianceMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms);

Window window;

Camera camera(
	glm::vec3(0.0f, 1.0f, 23.0f),
	glm::vec3(-10.0f, 9.0f, -33.0f),
	WORLD_UP,
	45.0f
);

Camera camHDRenv[NB_CUBEMAP_FACES] = {
	Camera(
		glm::vec3(0.0f),
		WORLD_RIGHT,
		-WORLD_UP,
		90.0f
	),
	Camera(
		glm::vec3(0.0f),
		-WORLD_RIGHT,
		-WORLD_UP,
		90.0f
	),
	Camera(
		glm::vec3(0.0f),
		WORLD_UP,
		WORLD_FWD,
		90.0f
	),
	Camera(
		glm::vec3(0.0f),
		-WORLD_UP,
		-WORLD_FWD,
		90.0f
	),
	Camera(
		glm::vec3(0.0f),
		WORLD_FWD,
		-WORLD_UP,
		90.0f
	),
	Camera(
		glm::vec3(0.0f),
		-WORLD_FWD,
		-WORLD_UP,
		90.0f
	)
};

glm::vec3 lightPositions[NB_POINT_LIGHTS] = {
	glm::vec3(-10.0f, 10.0f, 10.0f),
	glm::vec3(10.0f, 10.0f, 10.0f),
	glm::vec3(-10.0f, -10.0f, 10.0f),
	glm::vec3(10.0f, -10.0f, 10.0f),
};

glm::vec3 lightColors(300.0f);

glm::mat4 M, V, P, MVP;
glm::mat3 NM;

glm::vec3 sphereAlbedo(0.5f, 0.0f, 0.0f);
const float spacing = 2.5f;

double delta_time = 0.0;
double last_frame = 0.0f;
double current_frame;
float angle_x = 0.0f;
float angle_y = 0.0f;
float exposure = 4.5f;
unsigned int currentScene = 0;

const char* scenePaths[NB_SCENES] = {
	"./assets/HDR_maps/room/room.hdr",
	"./assets/HDR_maps/barcelona_rooftops/barcelona_rooftops.hdr",
	"./assets/HDR_maps/grand_canyon/grand_canyon.hdr",
	"./assets/HDR_maps/snow/snow.hdr",
	"./assets/HDR_maps/beach/beach.hdr"
};

const char* texUniform[TEX_PER_MAT] = {
	"material1.diffuseMap",
	"material1.specularMap",
	"material1.normalMap"
};

const char* roomTexLoc[TEX_PER_MAT] = {
	"./assets/textures/old_planks/old_planks_diff_2k.png",
	"./assets/textures/old_planks/old_planks_spec_2k.png",
	"./assets/textures/old_planks/old_planks_nor_2k.png"
};

const char* cubeTexLoc[TEX_PER_MAT] = {
	"./assets/textures/concrete/concrete_diff_2k.png",
	"./assets/textures/concrete/concrete_spec_2k.png",
	"./assets/textures/concrete/concrete_nor_2k.png"
};

std::vector<std::string> cubeMap = {
	"./assets/skybox/right.png",
	"./assets/skybox/left.png",
	"./assets/skybox/top.png",
	"./assets/skybox/bottom.png",
	"./assets/skybox/front.png",
	"./assets/skybox/back.png",
};

GLfloat cubeVerticesTangents[396];
// positions // normals // uvs // tangents
GLfloat cubeVertices[] = {
	// positions				 // normals					// uvs
	// back face
	 1.0f, -1.0f, -1.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	 1.0f, -1.0f, -1.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	// front face				 	    
	-1.0f, -1.0f,  1.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
	// left face					    
	-1.0f, -1.0f, -1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	-1.0f, -1.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	-1.0f,	1.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	-1.0f,	1.0f, -1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	// right face
	 1.0f, -1.0f,  1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	 1.0f, -1.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	// bottom face
	-1.0f, -1.0f, -1.0f,		 0.0f, -1.0f,  0.0f,		0.0f, 0.0f,
	 1.0f, -1.0f, -1.0f,		 0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,		 0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f,		 0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,		 0.0f, -1.0f,  0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,		 0.0f, -1.0f,  0.0f,		0.0f, 0.0f,
	// top face
	-1.0f,  1.0f,  1.0f,		 0.0f,  1.0f,  0.0f,		0.0f, 0.0f,
	 1.0f,  1.0f,  1.0f,		 0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,		 0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,		 0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,		 0.0f,  1.0f,  0.0f,		0.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,		 0.0f,  1.0f,  0.0f,		0.0f, 0.0f
};

// positions // normals // uvs // tangents
GLfloat roomVerticesTangents[396];
// winding order reversed (CW)
GLfloat roomVertices[] = {
	// positions				 // normals					// uvs
	// back face
	-1.0f, -1.0f, -1.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
	 1.0f, -1.0f, -1.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
	// front face	
	 1.0f, -1.0f,  1.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	-1.0f, -1.0f,  1.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	// left face	
	-1.0f, -1.0f,  1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	-1.0f, -1.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	// right face
	 1.0f, -1.0f, -1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	 1.0f, -1.0f, -1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	// bottom face
	-1.0f, -1.0f,  1.0f,		 0.0f,  1.0f,  0.0f,		0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,		 0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	 1.0f, -1.0f, -1.0f,		 0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	 1.0f, -1.0f, -1.0f,		 0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,		 0.0f,  1.0f,  0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,		 0.0f,  1.0f,  0.0f,		0.0f, 0.0f,
	// top face
	-1.0f,  1.0f, -1.0f,		 0.0f, -1.0f,  0.0f,		0.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,		 0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f,		 0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,		 0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,		 0.0f, -1.0f,  0.0f,		0.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,		 0.0f, -1.0f,  0.0f,		0.0f, 0.0f
};

GLfloat cubeMapVertices[] = {
	// positions	
	// back face
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	// front face	
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	// left face	
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	// right face
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	// bottom face
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	// top face
	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f
};

GLfloat quadVertices[] = {
	// positions		// uvs
	-1.0f, -1.0f,		0.0f, 0.0f,
	 1.0f, -1.0f,		1.0f, 0.0f,
	 1.0f,  1.0f,		1.0f, 1.0f,
	 1.0f,  1.0f,		1.0f, 1.0f,
	-1.0f,  1.0f,		0.0f, 1.0f,
	-1.0f, -1.0f,		0.0f, 0.0f
};