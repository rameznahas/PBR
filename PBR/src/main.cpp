#include <iostream>
#include <fstream>
#include <unordered_map>
#include "glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "main.h"
#include "Window.h"
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NB_POINT_LIGHTS 4

Window window;

Camera camera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -3.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	45.0f
);

Directional_Light directional_light(
	glm::vec3(-0.2f, -1.0f, -0.3f),
	glm::vec3(1.0f),
	glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f)
);

Point_Light point_lights[NB_POINT_LIGHTS] = {
	Point_Light(
		glm::vec3(0.7f, 0.2f, 2.0f),
		glm::vec3(1.0f, 0.6f, 1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(0.25f, 0.75f, 0.9f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(-4.0f, 2.0f, -12.0f),
		glm::vec3(1.0f, 0.2f, 0.2f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(0.0f, 0.0f, -3.0f),
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	)
};

Spotlight spotlight(
	glm::vec3(0.0f, 2.0f, 5.0f),
	glm::vec3(0.0f, -2.0f, -5.0f),
	glm::vec3(1.0f), 
	glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f),
	1.0f, 0.09f, 0.032f,
	12.5f, 15.0f
);

float delta_time = 0.0f;

int main() {
	if (!init()) return EXIT_FAILURE;

	Shader program("./shaders/vertex.shader", "./shaders/fragment.shader");
	Shader program_light("./shaders/light_vertex.shader", "./shaders/light_fragment.shader");

	GLfloat vertices[] = {
		// positions			// normals				// texture coords
		-0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,		0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	0.0f,  0.0f, -1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,		0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,		0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,		0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,		0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,		0.0f, 1.0f
	};

	glm::vec3 cube_positions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	GLuint VAO_light;

	glGenVertexArrays(1, &VAO_light);
	glBindVertexArray(VAO_light);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLuint container_diffuse_map = load_texture("./assets/images/container2.png");
	GLuint container_specular_map = load_texture("./assets/images/container2_specular.png");
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, container_diffuse_map);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, container_specular_map);

	program.use();
	program.set_int("material.diffuse", 0);
	program.set_int("material.specular", 1);


	float last_frame = 0.0f;
	
	// render loop
	while (!glfwWindowShouldClose(window())) {
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();
		program.set_vec3("dir_light.light.color", &directional_light.color[0]);
		program.set_vec3("dir_light.light.ambient", &directional_light.ambient[0]);
		program.set_vec3("dir_light.light.diffuse", &directional_light.diffuse[0]);
		program.set_vec3("dir_light.light.specular", &directional_light.specular[0]);
		program.set_vec3("dir_light.direction", &directional_light.direction[0]);

		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			std::string point_light = "point_lights[" + std::to_string(i) + "].";
			std::string color = point_light + "light.color";
			std::string ambient = point_light + "light.ambient";
			std::string diffuse = point_light + "light.diffuse";
			std::string specular = point_light + "light.specular";
			std::string kc = point_light + "light.kc";
			std::string kl = point_light + "light.kl";
			std::string kq = point_light + "light.kq";
			std::string position = point_light + "position";

			program.set_vec3(color.c_str(), &point_lights[i].color[0]);
			program.set_vec3(ambient.c_str(), &point_lights[i].ambient[0]);
			program.set_vec3(diffuse.c_str(), &point_lights[i].diffuse[0]);
			program.set_vec3(specular.c_str(), &point_lights[i].specular[0]);
			program.set_float(kc.c_str(), point_lights[i].kc);
			program.set_float(kl.c_str(), point_lights[i].kl);
			program.set_float(kq.c_str(), point_lights[i].kq);
			program.set_vec3(position.c_str(), &point_lights[i].position[0]);
		}

		program.set_vec3("spotlight.light.color", &spotlight.color[0]);
		program.set_vec3("spotlight.light.ambient", &spotlight.ambient[0]);
		program.set_vec3("spotlight.light.diffuse", &spotlight.diffuse[0]);
		program.set_vec3("spotlight.light.specular", &spotlight.specular[0]);
		program.set_float("spotlight.light.kc", spotlight.kc);
		program.set_float("spotlight.light.kl", spotlight.kl);
		program.set_float("spotlight.light.kq", spotlight.kq);
		program.set_vec3("spotlight.position", &camera.position[0]);
		program.set_vec3("spotlight.direction", &camera.forward[0]);
		program.set_float("spotlight.inner_cutoff", spotlight.inner_cutoff);
		program.set_float("spotlight.outer_cutoff", spotlight.outer_cutoff);
		
		program.set_float("material.shininess", 32.0f);

		program.set_vec3("cam_position", &camera.position[0]);

		glm::mat4 V = camera.get_view_matrix();
		glm::mat4 P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; ++i) {
			glm::mat4 M(1.0f);
			M = glm::translate(M, cube_positions[i]);
			M = glm::rotate(M, glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
			glm::mat4 MVP = P * V * M;
			glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(M)));
			program.set_mat4("M", &M[0][0]);
			program.set_mat4("MVP", &MVP[0][0]);
			program.set_mat3("normal_matrix", &normal_matrix[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		program_light.use();
		glBindVertexArray(VAO_light);
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			program_light.set_vec3("light_color", &point_lights[i].color[0]);

			glm::mat4 M(1.0f);
			M = glm::translate(M, point_lights[i].position);
			M = glm::scale(M, glm::vec3(0.1f));
			glm::mat4 MVP = P * V * M;
			program_light.set_mat4("MVP", &MVP[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// swap color buffer
		glfwSwapBuffers(window());
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	program.del();

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

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = Window(
		800, 600,
		glfwCreateWindow(800, 600, "PBR", nullptr, nullptr));

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
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.walk_around(camera.forward, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.walk_around(-camera.forward, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.walk_around(-camera.right, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.walk_around(camera.right, delta_time);
}

GLuint load_texture(const char* path) {
	GLuint texture;
	glGenTextures(1, &texture);

	int width, height, channels;
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if (data) {
		GLenum format;
		if (channels == 1) format = GL_RED;
		else if (channels == 2) format = GL_RG;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "ERROR::TEXTURE::LOADING" << std::endl;
	}

	stbi_image_free(data);

	return texture;
}