#pragma once
#include "glfw3.h"

struct Window {
public:
	Window() = default;
	Window(unsigned int w, unsigned int h, GLFWwindow* win)
		:
		width(w),
		height(h),
		window(win),
		center(w / 2.0f, h / 2.0f)
	{}
	GLFWwindow* operator()() const { return window; }

	unsigned int width;
	unsigned int height;
	glm::vec2 center;
private:
	GLFWwindow* window;
};