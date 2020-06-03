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

	Shader programModels("./shaders/modelsVertex.shader", "./shaders/modelsFragment.shader");
	Shader programShadowMap("./shaders/shadowMapVertex.shader", "./shaders/shadowMapFragment.shader");
	Shader programLight("./shaders/lightVertex.shader", "./shaders/lightFragment.shader");

	GLuint VAOcube, VBOcube;
	glGenVertexArrays(1, &VAOcube);
	glBindVertexArray(VAOcube);

	glGenBuffers(1, &VBOcube);
	glBindBuffer(GL_ARRAY_BUFFER, VBOcube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	GLuint VAOplane, VBOplane;
	glGenVertexArrays(1, &VAOplane);
	glBindVertexArray(VAOplane);

	glGenBuffers(1, &VBOplane);
	glBindBuffer(GL_ARRAY_BUFFER, VBOplane);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	GLuint texModels;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("./assets/images/wood.png", &width, &height, &channels, 0);
	if (data) {
		int format;
		if		(channels == 1) format = GL_RED;
		else if (channels == 2) format = GL_RG;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		glGenTextures(1, &texModels);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texModels);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		programModels.use();
		programModels.set_int("tex", 0);
	}
	else {
		std::cout << "ERROR::TEXTURE::LOADING" << std::endl;
	}

	GLuint UBOmodels, UBOlighting, UBOshadowMap;
	glGenBuffers(1, &UBOmodels);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOmodels);
	glBufferData(GL_UNIFORM_BUFFER, 240, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOmodels);

	glGenBuffers(1, &UBOshadowMap);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOshadowMap);
	glBufferData(GL_UNIFORM_BUFFER, 64, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBOshadowMap);

	glGenBuffers(1, &UBOlighting);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
	glBufferData(GL_UNIFORM_BUFFER, 96, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOlighting);

	unsigned int offset = 0;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &pointLight.color[0]);
	offset += sizeof(glm::vec3);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLfloat), &pointLight.kc);
	offset += sizeof(GLfloat);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &pointLight.ambient[0]);
	offset += sizeof(glm::vec3);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLfloat), &pointLight.kl);
	offset += sizeof(GLfloat);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &pointLight.diffuse[0]);
	offset += sizeof(glm::vec3);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLfloat), &pointLight.kq);
	offset += sizeof(GLfloat);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &pointLight.specular[0]);
	offset += sizeof(glm::vec4);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec4), &pointLight.position[0]);
	offset += sizeof(glm::vec4);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	GLuint FBOshadowMap, texShadowMap;
	glGenFramebuffers(1, &FBOshadowMap);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOshadowMap);
	glDrawBuffer(GL_NONE); // since we are not attaching a color buffer to the fbo
	glReadBuffer(GL_NONE); // since we are not attaching a color buffer to the fbo

	glGenTextures(1, &texShadowMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texShadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &glm::vec4(1.0f)[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	programModels.use();
	programModels.set_int("shadowMap", 1);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texShadowMap, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}
	
	double last_frame = 0.0f;
	double current_frame;

	V = shadowCam.get_view_matrix();
	P = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
	lightSpaceMatrix = P * V;

	P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

	float angle = glm::radians(60.0f);
	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glBindFramebuffer(GL_FRAMEBUFFER, FBOshadowMap);
		glViewport(0, 0, SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		programShadowMap.use();

		M[0] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
		M[0] = glm::rotate(M[0], angle, glm::normalize(glm::vec3(0.2f, 1.3f, 1.0f)));
		M[0] = glm::scale(M[0], glm::vec3(0.5f));
		MVP[0] = lightSpaceMatrix * M[0];
		normal_matrix[0] = glm::mat3(glm::transpose(glm::inverse(M[0])));

		M[1] = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 1.0f));
		M[1] = glm::rotate(M[1], angle, glm::normalize(glm::vec3(0.0f, 2.8f, 7.6f)));
		M[1] = glm::scale(M[1], glm::vec3(0.5f));
		MVP[1] = lightSpaceMatrix * M[1];
		normal_matrix[1] = glm::mat3(glm::transpose(glm::inverse(M[1])));

		M[2] = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 2.0f));
		M[2] = glm::rotate(M[2], angle, glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
		M[2] = glm::scale(M[2], glm::vec3(0.5f));
		MVP[2] = lightSpaceMatrix * M[2];
		normal_matrix[2] = glm::mat3(glm::transpose(glm::inverse(M[2])));

		M[3] = glm::mat4(1.0f);
		MVP[3] = lightSpaceMatrix * M[3];
		normal_matrix[3] = glm::mat3(glm::transpose(glm::inverse(M[3])));

		angle += glm::radians(15.0f * delta_time);

		renderShadowScene(VAOcube, VAOplane, UBOshadowMap);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);

		programModels.use();
		programModels.set_int("samples", samples);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texModels);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texShadowMap);

		V = camera.get_view_matrix();
		for (unsigned int i = 0; i < NB_MODELS; ++i) {
			MVP[i] = P * V * M[i];
		}

		renderScene(VAOcube, VAOplane, UBOlighting, UBOmodels, offset);

		glBindVertexArray(VAOcube);

		programLight.use();

		glm::mat4 M = glm::translate(glm::mat4(1.0f), pointLight.position);
		M = glm::scale(M, glm::vec3(0.1f));
		glm::mat4 MVP = P * V * M;
		programLight.set_mat4("MVP", &MVP[0][0]);
		programLight.set_vec3("lightColor", &pointLight.color[0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);


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
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = Window(
		800, 600,
		glfwCreateWindow(800, 600, "PBR", nullptr, nullptr)
	);

	if (!window()) {
		std::cout << "ERROR::GLFW::WINDOW::CREATION" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window());
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
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) samples += 2;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) samples -= 2;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	static double x, y;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetCursorPosCallback(window, nullptr);
		glfwGetCursorPos(window, &x, &y);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		camera.look_around(x, y);
		glfwSetCursorPos(window, ::window.center.x, ::window.center.y);
		mouse_button_down = true;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		camera.look_around(x, y);
		glfwSetCursorPosCallback(window, cursor_callback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mouse_button_down = false;
	}
}

void poll_mouse(Model& model) {
	if (mouse_button_down) {
		double x, y;
		glfwGetCursorPos(window(), &x, &y);

		angle_x += glm::radians(x - window.center.x) * 0.001f;
		angle_y += glm::radians(y - window.center.y) * 0.001f;

		glm::vec3 euler_angles(angle_y, angle_x, 0.0);
		glm::quat quat(euler_angles);
		model.M = glm::toMat4(quat);
	}
}

GLuint load_cubemap(std::vector<std::string>& paths) {
	GLuint cubemap;
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

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

	return cubemap;
}

void modelsBufferSubData(int idx) {
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &M[idx]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP[idx]);
	for (unsigned int i = 0; i < 3; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, nm_base_offset + i * sizeof(glm::vec4), sizeof(glm::vec4), &normal_matrix[idx][i][0]);
	}
}

void renderShadowScene(GLuint VAOcube, GLuint VAOplane, GLuint UBOshadowMap) {
	glBindVertexArray(VAOcube);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOshadowMap);

	for (unsigned int i = 0; i < NB_CUBES; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &MVP[i]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glBindVertexArray(VAOplane);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &MVP[3]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderScene(GLuint VAOcube, GLuint VAOplane, GLuint UBOlighting, GLuint UBOmodels, unsigned int offset) {
	glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &camera.position[0]);

	glBindBuffer(GL_UNIFORM_BUFFER, UBOmodels);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), &lightSpaceMatrix[0][0]);
	
	glBindVertexArray(VAOcube);
	for (unsigned int i = 0; i < NB_CUBES; ++i) {
		modelsBufferSubData(i);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(VAOplane);

	modelsBufferSubData(3);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}