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

	Shader programLightingRoom("./shaders/roomLightingVertex.shader", "./shaders/roomLightingFragment.shader");
	Shader programLightingCube("./shaders/cubeLightingVertex.shader", "./shaders/cubeLightingFragment.shader");
	Shader programPointShadow("./shaders/pointShadowVertex.shader", "./shaders/pointShadowFragment.shader");
	Shader programLight("./shaders/cubeLightingVertex.shader", "./shaders/lightFragment.shader");
	Shader programHDR("./shaders/hdrVertex.shader", "./shaders/hdrFragment.shader");

	computeTB();

	GLuint VAOroom, VBOroom, texRoom[NB_ROOM_TEX];
	glGenVertexArrays(1, &VAOroom);
	glBindVertexArray(VAOroom);

	glGenBuffers(1, &VBOroom);
	glBindBuffer(GL_ARRAY_BUFFER, VBOroom);
	glBufferData(GL_ARRAY_BUFFER, sizeof(roomVerticesTB), roomVerticesTB, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(11 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	programLightingRoom.use();
	texRoom[0] = loadTexture("./assets/images/brickwall/brickwall.jpg", GL_TEXTURE0);
	programLightingRoom.set_int("diffuseMap", 0);
	texRoom[1] = loadTexture("./assets/images/brickwall/brickwall_normal.jpg", GL_TEXTURE1);
	programLightingRoom.set_int("normalMap", 1);

	GLuint VAOcube, VBOcube, texCube;
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

	programLightingCube.use();
	texCube = loadTexture("./assets/images/wood.png", GL_TEXTURE0);
	programLightingCube.set_int("diffuseMap", 0);

	GLuint VAOhdr, VBOhdr;
	glGenVertexArrays(1, &VAOhdr);
	glBindVertexArray(VAOhdr);

	glGenBuffers(1, &VBOhdr);
	glBindBuffer(GL_ARRAY_BUFFER, VBOhdr);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	GLuint UBOtransforms, UBOlighting;
	glGenBuffers(1, &UBOtransforms);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);
	glBufferData(GL_UNIFORM_BUFFER, 11 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOtransforms);

	glGenBuffers(1, &UBOlighting);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
	glBufferData(GL_UNIFORM_BUFFER, 31 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOlighting);

	unsigned int lightingOffset = 0;
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &pointLights[i].color[0]);
		lightingOffset += sizeof(glm::vec3);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(GLfloat), &pointLights[i].kc);
		lightingOffset += sizeof(GLfloat);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &pointLights[i].ambient[0]);
		lightingOffset += sizeof(glm::vec3);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(GLfloat), &pointLights[i].kl);
		lightingOffset += sizeof(GLfloat);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &pointLights[i].diffuse[0]);
		lightingOffset += sizeof(glm::vec3);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(GLfloat), &pointLights[i].kq);
		lightingOffset += sizeof(GLfloat);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &pointLights[i].specular[0]);
		lightingOffset += sizeof(glm::vec4);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &pointLights[i].position[0]);
		lightingOffset += sizeof(glm::vec4);
	}
	glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset + sizeof(glm::vec3), sizeof(GLfloat), &farPlane);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	GLuint FBOpointShadow, texPointShadow[NB_POINT_LIGHTS];
	glGenFramebuffers(1, &FBOpointShadow);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpointShadow);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenTextures(NB_POINT_LIGHTS, texPointShadow);
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texPointShadow[i]);
		programLightingCube.use();
		programLightingCube.set_int(("pointShadow[" + std::to_string(i) + "]").c_str(), 2 + i);
		programLightingRoom.use();
		programLightingRoom.set_int(("pointShadow[" + std::to_string(i) + "]").c_str(), 2 + i);
		for (unsigned int j = 0; j < NB_CUBEMAP_FACES; ++j) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texPointShadow[0], 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}

	GLuint FBOhdr, texHDR, RBOhdr;
	glGenFramebuffers(1, &FBOhdr);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &texHDR);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHDR);
	programHDR.use();
	programHDR.set_int("hdrTex", 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window.width, window.height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &RBOhdr);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOhdr);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.width, window.height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHDR, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOhdr);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	double last_frame = 0.0f;
	double current_frame;

	P = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, farPlane);
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		for (unsigned int j = 0; j < NB_CUBEMAP_FACES; ++j) {
			pointShadowCams[j].position = pointLights[i].position;
			pointShadowLightSpaces[i][j] = P * pointShadowCams[j].get_view_matrix();
		}
	}
	
	P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 1000.0f);

	programPointShadow.use();
	programPointShadow.set_float("farPlane", farPlane);

	M[0] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
	M[0] = glm::scale(M[0], glm::vec3(15.0f));
	normalMatrix[0] = glm::transpose(glm::inverse(glm::mat3(M[0])));

	M[2] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
	M[2] = glm::scale(M[2], glm::vec3(0.1f));
	normalMatrix[2] = glm::transpose(glm::inverse(glm::mat3(M[2])));

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Mlight[i] = glm::translate(glm::mat4(1.0f), pointLights[i].position);
		Mlight[i] = glm::scale(Mlight[i], glm::vec3(0.2f));
	}

	float angle = glm::radians(60.0f);
	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		M[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -9.0f));
		M[1] = glm::rotate(M[1], angle, glm::vec3(1.0f));
		normalMatrix[1] = glm::transpose(glm::inverse(glm::mat3(M[1])));

		float time = (float)glfwGetTime();
		M[3] = glm::translate(glm::mat4(1.0f), 2.0f * glm::vec3(sin(time), 0.0f, cos(time)));
		M[3] = glm::scale(M[3], glm::vec3(0.2f));
		normalMatrix[3] = glm::transpose(glm::inverse(glm::mat3(M[3])));

		angle += (float)glm::radians(15.0f * delta_time);
		
		shadowPass(programPointShadow, FBOpointShadow, texPointShadow, VAOcube);

		glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		V = camera.get_view_matrix();
		VP = P * V;

		for (unsigned int i = 0; i < NB_MODELS; ++i) {
			MVP[i] = VP * M[i];
		}

		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			glActiveTexture(GL_TEXTURE2 + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texPointShadow[i]);
		}

		glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &camera.position[0]);
		
		glBindVertexArray(VAOroom);
		if (normalMapping) programLightingRoom.use();
		else programLightingCube.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texRoom[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texRoom[1]);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &M[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP[0]);
		for (unsigned int i = 0; i < 3; ++i) {
			glBufferSubData(GL_UNIFORM_BUFFER, nm_base_offset + i * sizeof(glm::vec4), sizeof(glm::vec3), &normalMatrix[0][i]);
		}
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(VAOcube);
		programLightingCube.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texCube);
		for (unsigned int i = 1; i < NB_MODELS; ++i) {
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &M[i]);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP[i]);
			for (unsigned int j = 0; j < 3; ++j) {
				glBufferSubData(GL_UNIFORM_BUFFER, nm_base_offset + j * sizeof(glm::vec4), sizeof(glm::vec3), &normalMatrix[i][j]);
			}
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		programLight.use();
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			programLight.set_vec3("lightColor", &pointLights[i].color[0]);
			MVPlight[i] = VP * Mlight[i];
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVPlight[i]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAOhdr);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texHDR);
		programHDR.use();
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
		800, 600,
		glfwCreateWindow(800, 600, "PBR", nullptr, nullptr)
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		normalMapping = !normalMapping;
	}
		
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

		angle_x += (float)glm::radians(x - window.center.x) * 0.001f;
		angle_y += (float)glm::radians(y - window.center.y) * 0.001f;

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

GLuint loadTexture(const char* path, GLenum tex) {
	GLuint texture;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if (data) {
		int format;
		if (channels == 1) format = GL_RED;
		else if (channels == 2) format = GL_RG;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		glGenTextures(1, &texture);
		glActiveTexture(tex);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

void shadowPass(Shader& programPointShadow, GLuint& FBOpointShadow, GLuint* texPointShadow, GLuint& VAOcube) {
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpointShadow);
	glViewport(0, 0, SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	programPointShadow.use();
	glBindVertexArray(VAOcube);

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		programPointShadow.set_vec3("lightPos", &pointLights[i].position[0]);
		for (unsigned int j = 0; j < NB_CUBEMAP_FACES; ++j) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, texPointShadow[i], 0);
			glClear(GL_DEPTH_BUFFER_BIT);

			for (unsigned int k = 0; k < NB_MODELS; ++k) {
				MVP[k] = pointShadowLightSpaces[i][j] * M[k];
				programPointShadow.set_mat4("MVP", &MVP[k][0][0]);
				programPointShadow.set_mat4("M", &M[k][0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
	}
}

void computeTB() {
	const unsigned int NB_FACES = 6;
	const unsigned int NB_POS = 3;
	unsigned int stride = 8;
	unsigned int nextFaceStride = 3 * stride;

	float* posPtr = roomVertices;
	float* uvPtr = posPtr + 6;

	glm::vec3 pos[NB_POS];
	glm::vec2 uv[NB_POS];

	glm::vec3 Ts[NB_FACES], Bs[NB_FACES];

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
		Bs[i] = detInverse * glm::vec2(-UV2[0], UV1[0]) * E;

		posPtr += nextFaceStride;
		uvPtr += nextFaceStride;
	}
	
	const unsigned int NB_VERTICES = 6 * NB_FACES;
	unsigned int start = 0;
	unsigned int end = start + stride;

	for (unsigned int i = 0; i < NB_VERTICES; ++i) {
		unsigned int offset = i * 6;
		for (unsigned int j = start; j < end; ++j) {
			roomVerticesTB[j + offset] = roomVertices[j];
		}
		start = end;
		end = start + stride;

		unsigned int idx = i / NB_FACES;
		roomVerticesTB[start + offset + 0] = Ts[idx][0];
		roomVerticesTB[start + offset + 1] = Ts[idx][1];
		roomVerticesTB[start + offset + 2] = Ts[idx][2];
		roomVerticesTB[start + offset + 3] = Bs[idx][0];
		roomVerticesTB[start + offset + 4] = Bs[idx][1];
		roomVerticesTB[start + offset + 5] = Bs[idx][2];
	}
}