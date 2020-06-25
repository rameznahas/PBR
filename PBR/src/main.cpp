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

	Shader programParallax("./shaders/quadVertex.shader", "./shaders/parallaxFragment.shader");

	GLuint VAOquad, VBOquad;
	GLuint quadAttribSizes[4] = { 3, 3, 3, 2 };
	initVertexAttributes(VAOquad, VBOquad, quadVertices, sizeof(quadVertices), 4, 11 * sizeof(GLfloat), quadAttribSizes);

	GLuint textures[3];
	glGenTextures(3, textures);
	for (unsigned int i = 0; i < 3; ++i) {
		textures[i] = loadTexture(texPaths[i], false);
	}

	M = glm::rotate(glm::mat4(1.0f), glm::radians(-80.0f), WORLD_RIGHT);
	NM = glm::mat3(glm::transpose(glm::inverse(M)));
	P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);


	programParallax.use();
	programParallax.set_vec3("wLightPos", &pointLight.position[0]);
	programParallax.set_int("albedoMap", 0);
	programParallax.set_int("normalMap", 1);
	programParallax.set_int("heightMap", 2);
	programParallax.set_mat4("M", &M[0][0]);
	programParallax.set_mat3("NM", &NM[0][0]);

	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		V = camera.get_view_matrix();
		MVP = P * V * M;
		programParallax.set_mat4("MVP", &MVP[0][0]);
		programParallax.set_vec3("wViewPos", &camera.position[0]);
		programParallax.set_int("parallax", parallax);
		programParallax.set_float("heightScale", heightScale);

		glBindVertexArray(VAOquad);
		for (unsigned int i = 0; i < 3; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
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
	glfwWindowHint(GLFW_SAMPLES, 16);

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
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) parallax = !parallax;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		heightScale += 0.001f;
		std::cout << "height scale: " << heightScale << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		heightScale -= 0.001f;
		std::cout << "height scale: " << heightScale << std::endl;
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

GLuint loadTexture(const char* path, bool gammaCorrection) {
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