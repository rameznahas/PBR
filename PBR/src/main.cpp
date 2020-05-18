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
//#include "Material.h"
#include "Model.h"

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
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(1.0f),
		glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
		1.0f, 0.09f, 0.032f
	),
	Point_Light(
		glm::vec3(-4.0f, 2.0f, -12.0f),
		glm::vec3(1.0f),
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

double delta_time = 0.0;

int main() {
	if (!init()) return EXIT_FAILURE;

	Shader program("./shaders/vertex.shader", "./shaders/fragment.shader");
	Shader program_light("./shaders/light_vertex.shader", "./shaders/light_fragment.shader");

	Model backpack("./assets/models/backpack/backpack.obj");

	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f
	};

	GLint indices[] = {
		0, 1, 2,
		0, 2, 3,
		4, 6, 5,
		4, 7, 6,
		1, 5, 2,
		5, 6, 2,
		0, 3, 4,
		3, 7, 4
	};

	GLuint VAO_light, VBO_light, EBO_light;
	glGenVertexArrays(1, &VAO_light);
	glGenBuffers(1, &VBO_light);
	glGenBuffers(1, &EBO_light);

	glBindVertexArray(VAO_light);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_light);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_light);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	double last_frame = 0.0f;
	
	// render loop
	while (!glfwWindowShouldClose(window())) {
		double current_frame = glfwGetTime();
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
			std::string idx = std::to_string(i);

			program.set_vec3(("point_lights[" + idx + "].light.color").c_str(), &point_lights[i].color[0]);
			program.set_vec3(("point_lights[" + idx + "].light.ambient").c_str(), &point_lights[i].ambient[0]);
			program.set_vec3(("point_lights[" + idx + "].light.diffuse").c_str(), &point_lights[i].diffuse[0]);
			program.set_vec3(("point_lights[" + idx + "].light.specular").c_str(), &point_lights[i].specular[0]);
			program.set_float(("point_lights[" + idx + "].light.kc").c_str(), point_lights[i].kc);
			program.set_float(("point_lights[" + idx + "].light.kl").c_str(), point_lights[i].kl);
			program.set_float(("point_lights[" + idx + "].light.kq").c_str(), point_lights[i].kq);
			program.set_vec3(("point_lights[" + idx + "].position").c_str(), &point_lights[i].position[0]);
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
		
		program.set_float("material.shininess", 128.0f);

		program.set_vec3("cam_position", &camera.position[0]);

		glm::mat4 M(1.0f);
		M = glm::translate(M, glm::vec3(0.0f));
		M = glm::scale(M, glm::vec3(1.0f));

		glm::mat4 V = camera.get_view_matrix();
		glm::mat4 P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

		glm::mat4 MVP = P * V * M;
		glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(M)));
		program.set_mat4("M", &M[0][0]);
		program.set_mat4("MVP", &MVP[0][0]);
		program.set_mat3("normal_matrix", &normal_matrix[0][0]);

		backpack.draw(program);
		

		program_light.use();
		glBindVertexArray(VAO_light);
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			program_light.set_vec3("light_color", &point_lights[i].color[0]);

			glm::mat4 M(1.0f);
			M = glm::translate(M, point_lights[i].position);
			M = glm::scale(M, glm::vec3(0.1f));
			glm::mat4 MVP = P * V * M;
			program_light.set_mat4("MVP", &MVP[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// swap color buffer
		glfwSwapBuffers(window());
		glfwPollEvents();
	}

	/*glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);*/
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