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

	Shader program_textured("./shaders/textured_vertex.shader", "./shaders/textured_fragment.shader");
	Shader program_mirror("./shaders/mirror_vertex.shader", "./shaders/mirror_fragment.shader");
	Shader program_light("./shaders/light_vertex.shader", "./shaders/light_fragment.shader");
	Shader program_skybox("./shaders/skybox_vertex.shader", "./shaders/skybox_fragment.shader");
	Shader program_gamma("./shaders/gamma_vertex.shader", "./shaders/gamma_fragment.shader");

	Model backpack_textured("./assets/models/backpack/backpack.obj");
	Model backpack_mirror("./assets/models/backpack_mirror/backpack.obj");
	backpack_mirror.M = glm::translate(backpack_mirror.M, backpack_mirror_pos);

	stbi_set_flip_vertically_on_load(false);

	GLuint VAO_light, VBO_light, EBO_light;
	glGenVertexArrays(1, &VAO_light);
	glGenBuffers(1, &VBO_light);
	glGenBuffers(1, &EBO_light);

	glBindVertexArray(VAO_light);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_light);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_light);

	glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(light_indices), light_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	GLuint VAO_skybox, VBO_skybox;
	glGenVertexArrays(1, &VAO_skybox);
	glGenBuffers(1, &VBO_skybox);

	glBindVertexArray(VAO_skybox);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_skybox);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	GLuint skybox = load_cubemap(cubemaps);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	program_skybox.use();
	program_skybox.set_int("skybox", 1);

	GLuint UBO_lighting;
	glGenBuffers(1, &UBO_lighting);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO_lighting);
	glBufferData(GL_UNIFORM_BUFFER, 416, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO_lighting);

	GLsizeiptr offset = 0;
	// we dont set directional light kc, kl, kq because it doesn't use them.
	// so we increment offset by PADDED_VEC3 directly
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directional_light.color[0]);
	offset += PADDED_VEC3;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directional_light.ambient[0]);
	offset += PADDED_VEC3;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directional_light.diffuse[0]);
	offset += PADDED_VEC3;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directional_light.specular[0]);
	offset += PADDED_VEC3;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &directional_light.direction[0]);
	offset += PADDED_VEC3;

	glm::mat4 M_point_lights[NB_POINT_LIGHTS];
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		M_point_lights[i] = glm::mat4(1.0f);
		M_point_lights[i] = glm::translate(M_point_lights[i], point_lights[i].position);
		M_point_lights[i] = glm::scale(M_point_lights[i], glm::vec3(0.1f));

		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &point_lights[i].color[0]);
		offset += sizeof(glm::vec3);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float), &point_lights[i].kc);
		offset += sizeof(float);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &point_lights[i].ambient[0]);
		offset += sizeof(glm::vec3);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float), &point_lights[i].kl);
		offset += sizeof(float);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &point_lights[i].diffuse[0]);
		offset += sizeof(glm::vec3);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float), &point_lights[i].kq);
		offset += sizeof(float);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &point_lights[i].specular[0]);
		offset += PADDED_VEC3;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &point_lights[i].position[0]);
		offset += PADDED_VEC3;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	GLuint FBO, dynamic_skybox, RBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &dynamic_skybox);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamic_skybox);
	program_mirror.use();
	program_mirror.set_int("skybox", 0);
	for (unsigned int i = 0; i < NB_CUBEMAP_FACES; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint VAOgamma, VBOgamma;
	glGenVertexArrays(1, &VAOgamma);
	glBindVertexArray(VAOgamma);

	glGenBuffers(1, &VBOgamma);
	glBindBuffer(GL_ARRAY_BUFFER, VBOgamma);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	GLuint FBOgamma, texGamma, RBOgamma;
	glGenFramebuffers(1, &FBOgamma);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOgamma);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);

	glGenTextures(1, &texGamma);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texGamma);
	program_gamma.use();
	program_gamma.set_int("tex", 2);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window.width, window.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &RBOgamma);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOgamma);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.width, window.height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texGamma, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOgamma);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	double last_frame = 0.0f;
	double current_frame;

	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glBindBuffer(GL_UNIFORM_BUFFER, UBO_lighting);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), &camera.position[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glViewport(0, 0, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION);
		P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);

		float time = (float)glfwGetTime();
		backpack_textured.M = glm::rotate(glm::mat4(1.0f), time, WORLD_RIGHT);
		backpack_textured.M = glm::rotate(backpack_textured.M, time, WORLD_UP);
		backpack_textured.M = glm::rotate(backpack_textured.M, time, WORLD_FWD);
		for (unsigned int i = 0; i < NB_CUBEMAP_FACES; ++i) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dynamic_skybox, 0);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			V = environment_mapping_cameras[i].get_view_matrix();
			MVP = P * V * backpack_textured.M;
			normal_matrix = glm::mat3(glm::transpose(glm::inverse(backpack_textured.M)));

			program_textured.use();
			program_textured.set_mat4("M", &backpack_textured.M[0][0]);
			program_textured.set_mat4("MVP", &MVP[0][0]);
			program_textured.set_mat3("normal_matrix", &normal_matrix[0][0]);

			backpack_textured.draw(program_textured);

			glBindVertexArray(VAO_skybox);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
			glDepthFunc(GL_LEQUAL);
			program_skybox.use();

			V = glm::mat4(glm::mat3(environment_mapping_cameras[i].get_view_matrix()));
			glm::mat4 VP = P * V;
			program_skybox.set_mat4("VP", &VP[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glDepthFunc(GL_LESS);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, FBOgamma);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		V = camera.get_view_matrix();
		P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);
		MVP = P * V * backpack_textured.M;
		normal_matrix = glm::mat3(glm::transpose(glm::inverse(backpack_textured.M)));

		program_textured.use();
		program_textured.set_mat4("M", &backpack_textured.M[0][0]);
		program_textured.set_mat4("MVP", &MVP[0][0]);
		program_textured.set_mat3("normal_matrix", &normal_matrix[0][0]);

		backpack_textured.draw(program_textured);

		MVP = P * V * backpack_mirror.M;
		normal_matrix = glm::mat3(glm::transpose(glm::inverse(backpack_mirror.M)));

		program_mirror.use();
		program_mirror.set_mat4("M", &backpack_mirror.M[0][0]);
		program_mirror.set_mat4("MVP", &MVP[0][0]);
		program_mirror.set_mat3("normal_matrix", &normal_matrix[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, dynamic_skybox);
		backpack_mirror.draw(program_mirror);

		/*glBindVertexArray(VAO_light);
		program_light.use();
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			program_light.set_vec3("light_color", &point_lights[i].color[0]);

			glm::mat4 MVP = P * V * M_point_lights[i];
			program_light.set_mat4("MVP", &MVP[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}*/

		glBindVertexArray(VAO_skybox);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
		glDepthFunc(GL_LEQUAL);
		program_skybox.use();

		V = glm::mat4(glm::mat3(camera.get_view_matrix()));
		P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

		glm::mat4 VP = P * V;
		program_skybox.set_mat4("VP", &VP[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAOgamma);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texGamma);

		program_gamma.use();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// swap color buffer
		glfwSwapBuffers(window());
		glfwPollEvents();
	}

	/*glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);*/
	program_textured.del();

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
	//glfwSetMouseButtonCallback(window(), mouse_button_callback);

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

void set_point_light_uniforms(Shader program, std::string idx, unsigned int i) {
	program.use();

	program.set_vec3(("point_lights[" + idx + "].light.color").c_str(), &point_lights[i].color[0]);
	program.set_vec3(("point_lights[" + idx + "].light.ambient").c_str(), &point_lights[i].ambient[0]);
	program.set_vec3(("point_lights[" + idx + "].light.diffuse").c_str(), &point_lights[i].diffuse[0]);
	program.set_vec3(("point_lights[" + idx + "].light.specular").c_str(), &point_lights[i].specular[0]);
	program.set_float(("point_lights[" + idx + "].light.kc").c_str(), point_lights[i].kc);
	program.set_float(("point_lights[" + idx + "].light.kl").c_str(), point_lights[i].kl);
	program.set_float(("point_lights[" + idx + "].light.kq").c_str(), point_lights[i].kq);
	program.set_vec3(("point_lights[" + idx + "].position").c_str(), &point_lights[i].position[0]);
}

void set_dir_light_uniforms(Shader program) {
	program.use();

	program.set_vec3("dir_light.light.color", &directional_light.color[0]);
	program.set_vec3("dir_light.light.ambient", &directional_light.ambient[0]);
	program.set_vec3("dir_light.light.diffuse", &directional_light.diffuse[0]);
	program.set_vec3("dir_light.light.specular", &directional_light.specular[0]);
	program.set_vec3("dir_light.direction", &directional_light.direction[0]);
}

void set_spotlight_uniforms(Shader program) {
	program.use();

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