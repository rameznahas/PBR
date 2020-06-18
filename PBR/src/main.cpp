#include <iostream>
#include <fstream>
#include <unordered_map>
#include "glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "main.h"
#include "stb_image.h"

int main() {
	if (!init()) return EXIT_FAILURE;

	Shader programPBRlighting("./shaders/lightingVertex.shader", "./shaders/pbrLightingFragment.shader");
	Shader programLight("./shaders/lightVertex.shader", "./shaders/lightFragment.shader");
	Shader programHDRskybox("./shaders/hdrSkyboxVertex.shader", "./shaders/hdrSkyboxFragment.shader");

	GLuint VAOsphere, VBOsphere, EBOsphere;
	size_t indexCount = initSphereVertices(VAOsphere, VBOsphere, EBOsphere);

	GLuint VAOquad, VBOquad;
	GLuint quadAttribSizes[2] = { 2, 2 };
	initVertexAttributes(VAOquad, VBOquad, quadVertices, sizeof(quadVertices), 2, 4 * sizeof(GLfloat), quadAttribSizes);

	GLuint VAOcubeMap, VBOcubeMap;
	GLuint cubeAttribeSize = 3;
	initVertexAttributes(VAOcubeMap, VBOcubeMap, cubeMapVertices, sizeof(cubeMapVertices), 1, 3 * sizeof(GLfloat), &cubeAttribeSize);

	GLuint UBOtransforms, UBOlighting;
	glGenBuffers(1, &UBOtransforms);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);
	glBufferData(GL_UNIFORM_BUFFER, 11 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOtransforms);

	glGenBuffers(1, &UBOlighting);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
	glBufferData(GL_UNIFORM_BUFFER, 9 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOlighting);

	unsigned int offset = 0;
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &lightPositions[i]);
		offset += sizeof(glm::vec4);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &lightColors[0]);
		offset += sizeof(glm::vec4);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	const unsigned int viewPosOffset = offset;

	GLuint texEnvMap[NB_SCENES], texIrradianceMap[NB_SCENES];
	for (unsigned int i = 0; i < NB_SCENES; ++i) {
		genEnvMap(scenePaths[i], texEnvMap[i], VAOcubeMap, UBOtransforms);
		genIrradianceMap(texEnvMap[i], texIrradianceMap[i], VAOcubeMap, UBOtransforms);
	}

	programPBRlighting.use();
	programPBRlighting.set_vec3("material1.albedo", &sphereAlbedo[0]);
	programPBRlighting.set_float("material1.ao", 1.0f);
	programPBRlighting.set_int("irradianceMap", 0);

	programHDRskybox.use();
	programHDRskybox.set_int("skybox", 0);

	P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

	const float xTrans = NB_SPHERE_COLS / 2.0f;
	const float yTrans = NB_SPHERE_ROWS / 2.0f;
	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		programPBRlighting.use();
		glBindVertexArray(VAOsphere);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texIrradianceMap[currentScene]);
		glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
		glBufferSubData(GL_UNIFORM_BUFFER, viewPosOffset, sizeof(glm::vec3), &camera.position[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);
		V = camera.get_view_matrix();

		for (unsigned int i = 0; i < NB_SPHERE_ROWS; ++i) {
			programPBRlighting.set_float("material1.metallic", (float)i / NB_SPHERE_ROWS);
			for (unsigned int j = 0; j < NB_SPHERE_COLS; ++j) {
				programPBRlighting.set_float("material1.roughness", glm::clamp((float)j / NB_SPHERE_COLS, 0.025f, 1.0f));
				M = glm::translate(glm::mat4(1.0f), spacing * glm::vec3(j - xTrans, i - yTrans, 0.0f));
				MVP = P * V * M;
				NM = glm::transpose(glm::inverse(glm::mat3(M)));
				setUBOtransforms(M, MVP, NM);
				glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
			}
		}

		programLight.use();
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			M = glm::translate(glm::mat4(1.0f), lightPositions[i]);
			M = glm::scale(M, glm::vec3(0.15f));
			MVP = P * V * M;
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP);
			programLight.set_vec3("lightColor", &lightColors[0]);
			glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
		}

		programHDRskybox.use();
		glBindVertexArray(VAOcubeMap);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texEnvMap[currentScene]);
		MVP = P * glm::mat4(glm::mat3(V));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP);
		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);

		// swap color buffer
		glfwSwapBuffers(window());
		glfwPollEvents();
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

bool init() {
	if (!glfwInit()) {
		std::cout << "ERROR::GLFW::INIT" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#if defined(FULLSCREEN)
	window = Window(
		1920, 1080,
		glfwCreateWindow(1920, 1080, "PBR", glfwGetPrimaryMonitor(), nullptr)
	);
#elif defined(SCREEN_GRAB)
	window = Window(
		1920, 1080,
		glfwCreateWindow(1920, 1080, "PBR", nullptr, nullptr)
	);
#else
	window = Window(
		1280, 720,
		glfwCreateWindow(1280, 720, "PBR", nullptr, nullptr)
	);
#endif

	if (!window()) {
		std::cout << "ERROR::GLFW::WINDOW::CREATION" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window());
#ifdef SCREEN_GRAB
	glfwSetWindowPos(window(), 0, 0);
#endif
	glfwSetInputMode(window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window(), framebuffer_size_callback);
	glfwSetKeyCallback(window(), key_callback);
	glfwSetCursorPosCallback(window(), cursor_callback);
	glfwSetScrollCallback(window(), scroll_callback);

	if (glewInit() != GLEW_OK) {
		std::cout << "ERROR::GLEW::INIT" << std::endl;
		glfwTerminate();
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	::window.width = width;
	::window.height = height;
	glViewport(0, 0, width, height);
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.look_around(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.zoom(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.walk_around(CAM_SPEED * camera.forward, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.walk_around(CAM_SPEED * -camera.forward, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.walk_around(CAM_SPEED * -camera.right, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.walk_around(CAM_SPEED * camera.right, delta_time);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentScene = 0;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentScene = 1;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentScene = 2;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) currentScene = 3;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) currentScene = 4;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		exposure += 0.5f;
		std::cout << exposure << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		exposure -= 0.5f;
		std::cout << exposure << std::endl;
	}
}

GLuint loadCubemap(std::vector<std::string>& paths) {
	GLuint cubemap;
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	stbi_set_flip_vertically_on_load(false);
	int width, height, channels;
	for (unsigned int i = 0; i < paths.size(); ++i) {
		unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);

		if (data) {
			GLenum format;
			if (channels == 1) format = GL_RED;
			else if (channels == 2) format = GL_RG;
			else if (channels == 3) format = GL_RGB;
			else if (channels == 4) format = GL_RGBA;
			
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		}
		else {
			std::cout << "ERROR::CUBEMAP::LOADING" << std::endl;
		}

		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return cubemap;
}

GLuint loadTexture(const char* path, GLenum tex, bool gammaCorrection) {
	GLuint texture;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if (data) {
		GLenum internalFormat, dataFormat;
		if (channels == 1) internalFormat = dataFormat = GL_RED;
		else if (channels == 2) internalFormat = dataFormat = GL_RG;
		else if (channels == 3) {
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (channels == 4) {
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glGenTextures(1, &texture);
		glActiveTexture(tex);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		std::cout << "ERROR::TEXTURE::LOADING" << std::endl;
	}
	stbi_image_free(data);

	return texture;
}

GLuint loadHDRmap(const char* path) {
	GLuint hdrMap;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(path, &width, &height, &channels, 0);
	if (data) {
		glGenTextures(1, &hdrMap);
		glBindTexture(GL_TEXTURE_2D, hdrMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "ERROR::TEXTURE::LOADING" << std::endl;
	}
	stbi_image_free(data);

	return hdrMap;
}

void computeVertTangents(float* vertices, float* to) {
	const unsigned int NB_FACES = 6;
	const unsigned int NB_POS = 3;
	const unsigned int NB_EXTRA_FLOATS = 3;
	unsigned int stride = 8;
	unsigned int nextFaceStride = 3 * stride;

	float* posPtr = vertices;
	float* uvPtr = posPtr + 6;

	glm::vec3 pos[NB_POS];
	glm::vec2 uv[NB_POS];
	glm::vec3 Ts[NB_FACES];

	for (unsigned int i = 0; i < NB_FACES; ++i) {
		for (unsigned int j = 0; j < NB_POS; ++j) {
			pos[j] = glm::vec3(posPtr[0], posPtr[1], posPtr[2]);
			uv[j] = glm::vec2(uvPtr[0], uvPtr[1]);
			posPtr += stride;
			uvPtr += stride;
		}

		glm::vec3 E1(pos[1] - pos[0]);
		glm::vec3 E2(pos[2] - pos[1]);
		glm::vec2 UV1(uv[1] - uv[0]);
		glm::vec2 UV2(uv[2] - uv[1]);

		float detInverse = 1.0f / (UV1[0] * UV2[1] - UV2[0] * UV1[1]);
		
		glm::mat3x2 E;
		E[0] = glm::vec2(E1[0], E2[0]);
		E[1] = glm::vec2(E1[1], E2[1]);
		E[2] = glm::vec2(E1[2], E2[2]);

		Ts[i] = detInverse * glm::vec2(UV2[1], -UV1[1]) * E;

		posPtr += nextFaceStride;
		uvPtr += nextFaceStride;
	}
	
	const unsigned int NB_VERTICES = 6 * NB_FACES;
	unsigned int start = 0;
	unsigned int end = start + stride;

	for (unsigned int i = 0; i < NB_VERTICES; ++i) {
		unsigned int offset = i * NB_EXTRA_FLOATS;
		for (unsigned int j = start; j < end; ++j) {
			to[j + offset] = vertices[j];
		}
		start = end;
		end = start + stride;

		unsigned int idx = i / NB_FACES;
		for (unsigned int j = 0; j < NB_EXTRA_FLOATS; ++j) {
			to[start + offset + j] = Ts[idx][j];
		}
	}
}

size_t initSphereVertices(GLuint& VAO, GLuint& VBO, GLuint& EBO) {
	const float PI = 3.14159265359f;
	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;

	for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
		float ySegment = (float)y / Y_SEGMENTS;
		float b = PI * ySegment;

		for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
			float xSegment = (float)x / X_SEGMENTS;

			float a = 2.0f * PI * xSegment;
			float xPos = cosf(a) * sinf(b);
			float yPos = cosf(b);
			float zPos = sinf(a) * sinf(b);

			positions.push_back(glm::vec3(xPos, yPos, zPos));
			normals.push_back(glm::vec3(xPos, yPos, zPos));
			uvs.push_back(glm::vec2(xSegment, ySegment));
		}
	}

	unsigned int x1 = X_SEGMENTS + 1;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
		if (y % 2 == 0) {
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
				indices.push_back(y * x1 + x);
				indices.push_back((y + 1) * x1 + x);
			}
		}
		else {
			for (int x = X_SEGMENTS; x >= 0; --x) {
				indices.push_back((y + 1) * x1 + x);
				indices.push_back(y * x1 + x);
			}
		}
	}

	std::vector<float> data;
	for (unsigned int i = 0; i < positions.size(); ++i) {
		data.push_back(positions[i].x);
		data.push_back(positions[i].y);
		data.push_back(positions[i].z);
		data.push_back(normals[i].x);
		data.push_back(normals[i].y);
		data.push_back(normals[i].z);
		data.push_back(uvs[i].x);
		data.push_back(uvs[i].y);
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
	unsigned int stride = 8 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indices.size();
}

void initVertexAttributes(GLuint& VAO, GLuint& VBO, GLfloat* data, GLuint size, GLuint nb_attrib, GLuint stride, GLuint* attribSizes) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	unsigned int offset = 0;
	for (unsigned int i = 0; i < nb_attrib; ++i) {
		glVertexAttribPointer(i, attribSizes[i], GL_FLOAT, GL_FALSE, stride, (void*)(offset * sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
		offset += attribSizes[i];
	}

	glBindVertexArray(0);
}

void setUBOtransforms(const glm::mat4& M, const glm::mat4& MVP, const glm::mat3& NM) {
	unsigned int offset = 0;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::mat4), &M[0][0]);
	offset += sizeof(glm::mat4);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::mat4), &MVP[0][0]);
	offset += sizeof(glm::mat4);
	for (unsigned int i = 0; i < 3; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &NM[i][0]);
		offset += sizeof(glm::vec4);
	}
}

void genEnvMap(const char* path, GLuint& texEnvMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms) {
	Shader programEnvMapGeneration("./shaders/cubemapVertex.shader", "./shaders/envMapGenerationFragment.shader");

	GLuint texHDRmap = loadHDRmap(path);

	GLuint FBO, RBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, HDR_ENV_RES, HDR_ENV_RES);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, HDR_ENV_RES, HDR_ENV_RES);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glGenTextures(1, &texEnvMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texEnvMap);
	for (unsigned int i = 0; i < NB_CUBEMAP_FACES; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, HDR_ENV_RES, HDR_ENV_RES, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}

	programEnvMapGeneration.use();
	glBindVertexArray(VAOcubeMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHDRmap);
	programEnvMapGeneration.set_int("equirectangularMap", 0);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);

	P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	for (unsigned int i = 0; i < NB_CUBEMAP_FACES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texEnvMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MVP = P * camHDRenv[i].get_view_matrix();
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	programEnvMapGeneration.del();
	glDeleteTextures(1, &texHDRmap);
	glDeleteFramebuffers(1, &FBO);
	glDeleteRenderbuffers(1, &RBO);
}

void genIrradianceMap(const GLuint& texEnvMap, GLuint& texIrradianceMap, const GLuint& VAOcubeMap, const GLuint& UBOtransforms) {
	Shader programIrradianceMapGeneration("./shaders/cubemapVertex.shader", "./shaders/irradianceMapGenerationFragment.shader");

	GLuint FBO, RBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, HDR_IRR_RES, HDR_IRR_RES);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, HDR_IRR_RES, HDR_IRR_RES);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glGenTextures(1, &texIrradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texIrradianceMap);
	for (unsigned int i = 0; i < NB_CUBEMAP_FACES; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, HDR_IRR_RES, HDR_IRR_RES, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}

	programIrradianceMapGeneration.use();
	glBindVertexArray(VAOcubeMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texEnvMap);
	programIrradianceMapGeneration.set_int("environmentMap", 0);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);

	P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	for (unsigned int i = 0; i < NB_CUBEMAP_FACES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texIrradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MVP = P * camHDRenv[i].get_view_matrix();
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	programIrradianceMapGeneration.del();
	glDeleteFramebuffers(1, &FBO);
	glDeleteRenderbuffers(1, &RBO);
}