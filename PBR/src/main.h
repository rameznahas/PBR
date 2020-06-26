#pragma once
#include <random>
#include "glfw3.h"
#include "Window.h"
#include "Camera.h"
#include "Light.h"
#include "Shader.h"
#include "Model.h"

#define DEBUGGING
//#define FULLSCREEN
//#define SCREEN_GRAB

#ifdef DEBUGGING
	#define ENV_MAP_RES 1024
	#define IRR_MAP_RES 32
	#define PRE_FILTERED_ENV_MAP_RES 128
	#define BRDF_INT_MAP_RES 256
#else
	#define ENV_MAP_RES 4096
	#define IRR_MAP_RES 128
	#define PRE_FILTERED_ENV_MAP_RES 512
	#define BRDF_INT_MAP_RES 1024
#endif

#define CAM_SPEED 2.0f
#define NB_POINT_LIGHTS 4
#define NB_CUBEMAP_FACES 6
#define NB_SPHERE_ROWS 7
#define NB_SPHERE_COLS 7
#define NB_HDR_TEX 2
#define NB_SCENES 6
#define NB_TEX_SPHERE 5
#define TEX_PER_MAT 6
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
size_t initSphereVertices(GLuint& VAO, GLuint& VBO, GLuint& EBO);
void initVertexAttributes(GLuint& VAO, GLuint& VBO, GLfloat* data, GLuint size, GLuint nb_attrib, GLuint stride, GLuint* attribSizes);
void setUBOtransforms(const glm::mat4& M, const glm::mat4& MVP, const glm::mat3& NM);
void createTextureCubemap(GLuint& tex, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint textureWrapping, GLint minFiltering, GLint magFiltering, bool mipmap);
void drawToCubemapFaces(const GLuint& tex, GLint level);
void genEnvMap(const GLuint& FBO, const GLuint& RBO, const char* path, GLuint& texEnvMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms);
void genIrradianceMap(const GLuint& FBO, const GLuint& RBO, const GLuint& texEnvMap, GLuint& texIrradianceMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms);
void genPreFilteredEnvMap(const GLuint& FBO, const GLuint& RBO, const GLuint& texEnvMap, GLuint& texPreFilteredEnvMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms);
void genBRDFintegrationMap(const GLuint& FBO, const GLuint& RBO, GLuint& texBRDFintegrationMap, const GLuint& VAOquad);

Window window;

Camera camera(
	glm::vec3(0.0f, 1.0f, 23.0f),
	glm::vec3(-10.0f, 9.0f, -33.0f),
	WORLD_UP,
	45.0f
);

glm::mat4 MVPenvCam[NB_CUBEMAP_FACES];
Camera camEnvMap[NB_CUBEMAP_FACES] = {
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

glm::vec3 sphereAlbedo(0.5f, 0.5f, 0.5f);
const float spacing = 2.5f;

double delta_time = 0.0;
double last_frame = 0.0f;
double current_frame;
float angle_x = 0.0f;
float angle_y = 0.0f;
float exposure = 0.5f;
unsigned int currentScene = 0;
bool textured = false;
float heightScales[NB_TEX_SPHERE] = { 0.01f,0.02f,0.05f,0.01f,0.05f };
bool parallax = false;

const char* scenePaths[NB_SCENES] = {
	"./assets/HDR_maps/circus_arena.hdr",
	"./assets/HDR_maps/fireplace.hdr",
	"./assets/HDR_maps/red_wall.hdr",
	"./assets/HDR_maps/the_sky_is_on_fire.hdr",
	"./assets/HDR_maps/urban_street.hdr",
	"./assets/HDR_maps/winter_evening.hdr"
};

const char* texUniform[TEX_PER_MAT] = {
	"material1.albedo",
	"material1.normal",
	"material1.metallic",
	"material1.roughness",
	"material1.ao",
	"material1.height"
};

const char* texLoc[NB_TEX_SPHERE][TEX_PER_MAT] = {
	{
		"./assets/textures/texture1/d.png",
		"./assets/textures/texture1/n.png",
		"./assets/textures/texture1/m.png",
		"./assets/textures/texture1/r.png",
		"./assets/textures/texture1/a.png",
		"./assets/textures/texture1/h.png"
	},
	{
		"./assets/textures/texture2/d.png",
		"./assets/textures/texture2/n.png",
		"./assets/textures/texture2/m.png",
		"./assets/textures/texture2/r.png",
		"./assets/textures/texture2/a.png",
		"./assets/textures/texture2/h.png"
	},
	{
		"./assets/textures/texture3/d.png",
		"./assets/textures/texture3/n.png",
		"./assets/textures/texture3/m.png",
		"./assets/textures/texture3/r.png",
		"./assets/textures/texture3/a.png",
		"./assets/textures/texture3/h.png"
	},
	{
		"./assets/textures/texture4/d.png",
		"./assets/textures/texture4/n.png",
		"./assets/textures/texture4/m.png",
		"./assets/textures/texture4/r.png",
		"./assets/textures/texture4/a.png",
		"./assets/textures/texture4/h.png"
	},
	{
		"./assets/textures/texture5/d.png",
		"./assets/textures/texture5/n.png",
		"./assets/textures/texture5/m.png",
		"./assets/textures/texture5/r.png",
		"./assets/textures/texture5/a.png",
		"./assets/textures/texture5/h.png"
	}
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