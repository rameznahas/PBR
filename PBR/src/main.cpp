#include "glew.h"
#include "glfw3.h"
#include <iostream>

unsigned int WNDW_WIDTH = 800;
unsigned int WNDW_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	WNDW_WIDTH = width;
	WNDW_HEIGHT = height;
	glViewport(0, 0, WNDW_WIDTH, WNDW_HEIGHT);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}

int main() {
	const char* vertex_shader_src = "#version 430 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(pos, 1.0f);\n"
		"}\0";

	const char* fragment_shader_src = "#version 430 core\n"
		"out vec4 fragment_color;\n"
		"void main()\n"
		"{\n"
		"   fragment_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";

	if (!glfwInit()) {
		std::cout << "ERROR::GLFW::INIT" << std::endl;
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "PBR", nullptr, nullptr);

	if (!window) {
		std::cout << "ERROR::GLFW::WINDOW::CREATION" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (glewInit() != GLEW_OK) {
		std::cout << "ERROR::GLEW::INIT" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(window, key_callback);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_src, nullptr);
	glCompileShader(vertex_shader);

	int success;
	char log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, sizeof(log), nullptr, log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION" << std::endl << log << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, sizeof(log), nullptr, log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION" << std::endl << log << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		std::cout << "ERROR::PROGRAM::SHADER::LINKING" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);


	float vertices[] = {
		 0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glUseProgram(program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glBindVertexArray(0);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// swap color buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}