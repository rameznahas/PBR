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

	Shader programGeometryPass("./shaders/geometryPassVertex.shader", "./shaders/geometryPassFragment.shader");
	Shader programLightingPass("./shaders/lightingPassVertex.shader", "./shaders/lightingPassFragment.shader");
	Shader programPointShadow("./shaders/pointShadowVertex.shader", "./shaders/pointShadowFragment.shader");
	Shader programLight("./shaders/lightVertex.shader", "./shaders/lightFragment.shader");
	Shader programBloom("./shaders/toneMappingVertex.shader", "./shaders/bloomFragment.shader");
	Shader programToneMapping("./shaders/toneMappingVertex.shader", "./shaders/toneMappingFragment.shader");
	Shader programSkybox("./shaders/skyboxVertex.shader", "./shaders/skyboxFragment.shader");

	Model backpack("./assets/models/backpack/backpack.obj", false);
	Model lightBackpack("./assets/models/backpack/backpack.obj", true);

	GLuint VAOroom, VBOroom, texRoom[TEX_PER_MAT]; 
	GLuint attribSizes[4] = { 3, 3, 2, 3 };
	computeVertTangents(roomVertices, roomVerticesTangents);
	initVertexAttributes(VAOroom, VBOroom, roomVerticesTangents, sizeof(roomVerticesTangents), 4, 11 * sizeof(GLfloat), attribSizes);
	initTextures(programGeometryPass, texRoom, roomTexLoc);

	GLuint VAOcube, VBOcube, texCube[TEX_PER_MAT];
	computeVertTangents(cubeVertices, cubeVerticesTangents);
	initVertexAttributes(VAOcube, VBOcube, cubeVerticesTangents, sizeof(cubeVerticesTangents), 4, 11 * sizeof(GLfloat), attribSizes);
	initTextures(programGeometryPass, texCube, cubeTexLoc);

	GLuint VAOquad, VBOquad;
	GLuint quadAttribSizes[2] = { 2, 2 };
	initVertexAttributes(VAOquad, VBOquad, quadVertices, sizeof(quadVertices), 2, 4 * sizeof(GLfloat), quadAttribSizes);

	GLuint VAOskbyox, VBOskybox, texSkybox;
	GLuint skyboxAttribSize = 3;
	initVertexAttributes(VAOskbyox, VBOskybox, skyboxVertices, sizeof(skyboxVertices), 1, 3 * sizeof(GLfloat), &skyboxAttribSize);
	texSkybox = load_cubemap(cubeMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
	programSkybox.use();
	programSkybox.set_int("skybox", 0);

	GLuint UBOtransforms, UBOlighting;
	glGenBuffers(1, &UBOtransforms);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);
	glBufferData(GL_UNIFORM_BUFFER, 11 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOtransforms);

	glGenBuffers(1, &UBOlighting);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
	glBufferData(GL_UNIFORM_BUFFER, 46 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
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
	unsigned int base = GL_TEXTURE0 + TEX_PER_MAT;
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		glActiveTexture(base + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texPointShadow[i]);

		programLightingPass.use();
		programLightingPass.set_int(("pointShadow[" + std::to_string(i) + "]").c_str(), TEX_PER_MAT + i);

		for (unsigned int j = 0; j < NB_CUBEMAP_FACES; ++j) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, SHADOWMAP_RES, SHADOWMAP_RES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
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

	GLuint FBOhdr, texHDR[NB_HDR_TEX], RBOhdr;
	glGenFramebuffers(1, &FBOhdr);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);
	GLuint drawBuffs[NB_HDR_TEX] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(NB_HDR_TEX, drawBuffs);

	glGenTextures(NB_HDR_TEX, texHDR);
	programToneMapping.use();
	const char* samplers[NB_HDR_TEX] = { "hdrTex", "bloom" };
	for (unsigned int i = 0; i < NB_HDR_TEX; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texHDR[i]);
		programToneMapping.set_int(samplers[i], i);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window.width, window.height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texHDR[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &RBOhdr);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOhdr);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.width, window.height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOhdr);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}

	GLuint FBObloom;
	glGenFramebuffers(1, &FBObloom);
	glBindFramebuffer(GL_FRAMEBUFFER, FBObloom);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHDR[1], 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE" << std::endl;
	}

	GLuint gBuffer, gPosition, gNormal, gColorSpec, gRBO;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window.width, window.height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window.width, window.height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window.width, window.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &gRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.width, window.height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRBO);
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
	
	P = glm::perspective(glm::radians(camera.fov), (float)window.width / window.height, 0.1f, 100.0f);

	programPointShadow.use();
	programPointShadow.set_float("farPlane", farPlane);

	M[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
	M[1] = glm::scale(M[1], glm::vec3(7.5f));
	normalMatrix[1] = glm::transpose(glm::inverse(glm::mat3(M[1])));

	M[2] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
	M[2] = glm::scale(M[2], glm::vec3(0.1f));
	normalMatrix[2] = glm::transpose(glm::inverse(glm::mat3(M[2])));

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Mlight[i] = glm::translate(glm::mat4(1.0f), pointLights[i].position);
		Mlight[i] = glm::scale(Mlight[i], glm::vec3(0.1f));
	}

	float angle = glm::radians(60.0f);
	// render loop
	while (!glfwWindowShouldClose(window())) {
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		M[0] = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -5.0f, 5.0f));
		M[0] = glm::rotate(M[0], angle, WORLD_UP);
		M[0] = glm::scale(M[0], glm::vec3(0.5f));
		normalMatrix[0] = glm::transpose(glm::inverse(glm::mat3(M[0])));

		M[3] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		M[3] = glm::rotate(M[3], angle, glm::vec3(1.0f));
		normalMatrix[3] = glm::transpose(glm::inverse(glm::mat3(M[3])));

		float time = (float)glfwGetTime();
		M[4] = glm::translate(glm::mat4(1.0f), 2.5f * glm::vec3(sin(time), 0.0f, cos(time)));
		M[4] = glm::scale(M[4], glm::vec3(0.2f));
		normalMatrix[4] = glm::transpose(glm::inverse(glm::mat3(M[4])));

		angle += (float)glm::radians(15.0f * delta_time);

		shadowPass(programPointShadow, FBOpointShadow, texPointShadow, lightBackpack, VAOroom, VAOcube);

		V = camera.get_view_matrix();
		VP = P * V;
		for (unsigned int i = 0; i < NB_MODELS; ++i) {
			MVP[i] = VP * M[i];
		}

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		programGeometryPass.use();
		glBindBuffer(GL_UNIFORM_BUFFER, UBOlighting);
		glBufferSubData(GL_UNIFORM_BUFFER, lightingOffset, sizeof(glm::vec3), &camera.position[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, UBOtransforms);

		initUBOtransform(M[0], MVP[0], normalMatrix[0]);
		backpack.draw(programGeometryPass, GL_TEXTURE0, "Map");

		glBindVertexArray(VAOroom);
		bindSimpleModelTexs(texRoom);
		initUBOtransform(M[1], MVP[1], normalMatrix[1]);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(VAOcube);
		bindSimpleModelTexs(texCube);
		for (unsigned int i = 2; i < NB_MODELS; ++i) {
			initUBOtransform(M[i], MVP[i], normalMatrix[i]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		programLightingPass.use();

		glBindVertexArray(VAOquad);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		programLightingPass.set_int("worldPositions", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		programLightingPass.set_int("worldNorms", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gColorSpec);
		programLightingPass.set_int("colorSpecs", 2);
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			glActiveTexture(base + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texPointShadow[i]);
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOhdr);
		glBlitFramebuffer(0, 0, window.width, window.height, 0, 0, window.width, window.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		programLight.use();
		glBindVertexArray(VAOcube);
		for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
			programLight.set_vec3("lightColor", &pointLights[i].color[0]);
			MVPlight[i] = VP * Mlight[i];
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVPlight[i]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		/*programSkybox.use();
		glBindVertexArray(VAOskbyox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
		V = glm::mat4(glm::mat3(V));
		VP = P * V;
		programSkybox.set_mat4("VP", &VP[0][0]);
		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);*/

		glBindFramebuffer(GL_FRAMEBUFFER, FBObloom);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		programBloom.use();
		glBindVertexArray(VAOquad);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texHDR[1]);
		programBloom.set_int("bloom", 0);
		bool horizontal = true;
		for (unsigned int i = 0; i < 10; ++i) {
			programBloom.set_int("horizontal", horizontal);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			horizontal != horizontal;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.width, window.height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		programToneMapping.use();
		for (unsigned int i = 0; i < NB_HDR_TEX; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, texHDR[i]);
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOhdr);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, window.width, window.height, 0, 0, window.width, window.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		programSkybox.use();
		glBindVertexArray(VAOskbyox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
		V = glm::mat4(glm::mat3(V));
		VP = P * V;
		programSkybox.set_mat4("VP", &VP[0][0]);
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
	glEnable(GL_CULL_FACE);

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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) normalMapping = !normalMapping;
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

void shadowPass(Shader& programPointShadow, GLuint& FBOpointShadow, GLuint* texPointShadow, Model& model, GLuint& VAOroom, GLuint& VAOcube) {
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpointShadow);
	glViewport(0, 0, SHADOWMAP_RES, SHADOWMAP_RES);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	programPointShadow.use();

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		programPointShadow.set_vec3("lightPos", &pointLights[i].position[0]);
		for (unsigned int j = 0; j < NB_CUBEMAP_FACES; ++j) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, texPointShadow[i], 0);
			glClear(GL_DEPTH_BUFFER_BIT);

			MVP[0] = pointShadowLightSpaces[i][j] * M[0];
			programPointShadow.set_mat4("MVP", &MVP[0][0][0]);
			programPointShadow.set_mat4("M", &M[0][0][0]);
			model.draw(programPointShadow, GL_TEXTURE0);

			glBindVertexArray(VAOroom);
			MVP[1] = pointShadowLightSpaces[i][j] * M[1];
			programPointShadow.set_mat4("MVP", &MVP[1][0][0]);
			programPointShadow.set_mat4("M", &M[1][0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			glBindVertexArray(VAOcube);
			for (unsigned int k = 2; k < NB_MODELS; ++k) {
				MVP[k] = pointShadowLightSpaces[i][j] * M[k];
				programPointShadow.set_mat4("MVP", &MVP[k][0][0]);
				programPointShadow.set_mat4("M", &M[k][0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
	}
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

void initUBOtransform(glm::mat4&M, glm::mat4& MVP, glm::mat3& normalMatrix) {
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &M);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &MVP);
	for (unsigned int i = 0; i < 3; ++i) {
		glBufferSubData(GL_UNIFORM_BUFFER, nm_base_offset + i * sizeof(glm::vec4), sizeof(glm::vec3), &normalMatrix[i]);
	}
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

void initTextures(Shader& program, GLuint* texs, const char** tex_locations) {
	program.use();
	for (unsigned int i = 0; i < TEX_PER_MAT; ++i) {
		texs[i] = loadTexture(tex_locations[i], GL_TEXTURE0 + i);
		program.set_int(texUniform[i], i);
	}
}

void bindSimpleModelTexs(GLuint* tex) {
	for (unsigned int i = 0; i < TEX_PER_MAT; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]);
	}
}