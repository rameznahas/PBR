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

	GLuint tex;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("./assets/images/wood.png", &width, &height, &channels, 0);
	if (data) {
		int format;
		if		(channels == 1) format = GL_RED;
		else if (channels == 2) format = GL_RG;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		glGenTextures(1, &tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
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

	GLuint UBOmodels, UBOlighting;
	glGenBuffers(1, &UBOmodels);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOmodels);
	glBufferData(GL_UNIFORM_BUFFER, 176, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOmodels);

	glGenBuffers(1, &UBOlighting);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
	glBufferData(GL_UNIFORM_BUFFER, 96, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOlighting);

	unsigned int offset = 0;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directionalLight.color[0]);
	offset += sizeof(glm::vec3);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLfloat), &directionalLight.kc);
	offset += sizeof(GLfloat);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directionalLight.ambient[0]);
	offset += sizeof(glm::vec3);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLfloat), &directionalLight.kl);
	offset += sizeof(GLfloat);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directionalLight.diffuse[0]);
	offset += sizeof(glm::vec3);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLfloat), &directionalLight.kq);
	offset += sizeof(GLfloat);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directionalLight.specular[0]);
	offset += sizeof(glm::vec4);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec4), &directionalLight.direction[0]);
	offset += sizeof(glm::vec4);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	double last_frame = 0.0f;
	double current_frame;

	P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		programModels.use();

		glBindVertexArray(VAOcube);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
		
		V = camera.get_view_matrix();
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &camera.position[0]);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOmodels);

		M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
		M = glm::scale(M, glm::vec3(0.5f));
		MVP = P * V * M;
		normal_matrix = glm::mat3(glm::transpose(glm::inverse(M)));
		modelsBufferSubData();
		glDrawArrays(GL_TRIANGLES, 0, 36);

		M = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 1.0f));
		M = glm::scale(M, glm::vec3(0.5f));
		MVP = P * V * M;
		normal_matrix = glm::mat3(glm::transpose(glm::inverse(M)));
		modelsBufferSubData();
		glDrawArrays(GL_TRIANGLES, 0, 36);

		M = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 2.0f));
		M = glm::rotate(M, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
		M = glm::scale(M, glm::vec3(0.5f));
		normal_matrix = glm::mat3(glm::transpose(glm::inverse(M)));
		MVP = P * V * M;
		modelsBufferSubData();
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(VAOplane);

		M = glm::mat4(1.0f);
		MVP = P * V * M;
		normal_matrix = glm::mat3(glm::transpose(glm::inverse(M)));
		modelsBufferSubData();
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

void modelsBufferSubData() {
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &M[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP[0][0]);
	for (unsigned int i = 0; i < 3; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, nm_base_offset + i * sizeof(glm::vec4), sizeof(glm::vec4), &normal_matrix[i][0]);
	}
}