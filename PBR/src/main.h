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

#define CAM_SPEED 1.5f
#define NB_SSAO_SAMPLES 64
#define NB_POINT_LIGHTS 9
#define NB_CUBEMAP_FACES 6
#define NB_MODELS 5
#define NB_HDR_TEX 2
#define NB_BLOOM_TEX 2
#define NB_TRANSF_UNIFORMS 4
#define SSAO_NOISE_TEX_RES 4
#define TEX_PER_MAT 3
#define SHADOWMAP_RES 2048
#define WORLD_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define WORLD_FWD glm::vec3(0.0f, 0.0f, 1.0f)

bool init();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint load_cubemap(std::vector<std::string>& paths);
GLuint loadTexture(const char* path, GLenum tex, bool gammaCorrection);
void shadowPass(Shader& programPointShadow, GLuint& FBOpointShadow, GLuint* texPointShadow, Model& model, GLuint& VAOroom, GLuint& VAOcube);
void ssaoPass(GLuint& FBOssao, GLuint& FBOssaoBlur, GLuint& VAOquad, GLuint& gPosition, GLuint& gNormal, GLuint& texNoise, GLuint& texSSAO, Shader& programSSAO, Shader& programSSAOblur);
void computeVertTangents(float* vertices, float* to);
void initUBOtransform(glm::mat4&M, glm::mat4& MVP, glm::mat3& normalMatrix);
void initVertexAttributes(GLuint& VAO, GLuint& VBO, GLfloat* data, GLuint size, GLuint nb_attrib, GLuint stride, GLuint* attribSizes);
void initTextures(Shader& program, GLuint* texs, const char** tex_locations);
void bindSimpleModelTexs(GLuint* tex);
void initSSAOKernel();
void initSSAONoise();
float lerp(float a, float b, float c);

Window window;

Camera camera(
	glm::vec3(-6.5f, 6.0f, 6.5f),
	glm::vec3(6.5f, -6.0f, -6.5f),
	WORLD_UP,
	45.0f
);

Camera pointShadowCams[NB_CUBEMAP_FACES] = {
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

glm::mat4 pointShadowLightSpaces[NB_POINT_LIGHTS][NB_CUBEMAP_FACES];
glm::mat4 envMapSpaces[NB_CUBEMAP_FACES];
float farPlane = 50.0f;

Point_Light pointLights[NB_POINT_LIGHTS] = {
	Point_Light(
		glm::vec3(5.0f, -5.0f, 2.5f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(0.0f, 3.0f, 0.0f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(6.5f, 6.5f, -6.5f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(-6.5f, 6.5f, 0.0f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(0.0f, 2.0f, -3.25f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(6.5f, 6.5f, 0.0f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(-6.5f, 6.5f, 6.5f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(0.0f, 6.5f, 6.5f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	),
	Point_Light(
		glm::vec3(6.5f, 6.5f, 6.5f),
		glm::vec3(5.0f),
		glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(1.0f),
		1.0f, 0.7f, 1.8f
	)
};

glm::mat4 M[NB_MODELS], V, P, VP, MVP[NB_MODELS];
glm::mat4 Mlight[NB_POINT_LIGHTS], MVPlight[NB_POINT_LIGHTS];
glm::mat3 normalMatrix[NB_MODELS];

std::size_t UBOtransformsOffsets[NB_TRANSF_UNIFORMS] = {
	0,
	sizeof(glm::mat4),
	sizeof(glm::mat4) * 2,
	sizeof(glm::mat4) * 3
};

std::vector<glm::vec3> ssaoKernel;
std::vector<glm::vec3> ssaoNoise;
std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);
std::default_random_engine generator;

unsigned int shadowBaseTexUnit = GL_TEXTURE4;

double delta_time = 0.0;
double last_frame = 0.0f;
double current_frame;
float angle_x = 0.0f;
float angle_y = 0.0f;
float exposure = 4.5f;
bool ssao = false;
int ssaoPower = 10;
float ssaoRadius = 8.0f;

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

GLfloat skyboxVertices[] = {
	// positions		
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