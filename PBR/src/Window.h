#pragma once
#include "glfw3.h"

struct Window {
public:
	Window() = default;
	Window(unsigned int w, unsigned int h, GLFWwindow* win)
		:
		width(w),
		height(h),
		window(win)
	{}
	GLFWwindow* operator()() const { return window; }

	unsigned int width;
	unsigned int height;
private:
	GLFWwindow* window;
};