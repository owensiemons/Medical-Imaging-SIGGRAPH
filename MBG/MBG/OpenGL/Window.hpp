#pragma once

#include <cassert>
#include <cstring>
#include <glad/glad.h>
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include <GL/glext.h>
#include <iostream>

#include "FrameGraph.hpp"

namespace MBG {

class Window
{
public:
	Window(int width, int height, std::string window_name);

	bool isClosed() const;

	inline int getWidth() const { return width_; };
	inline int getHeight() const { return height_; };

	inline GLFWwindow* getWindow() const { return window; };

	~Window();

private:
	static void framebuffer_size_callback_static(GLFWwindow* window, int width, int height);
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	void startGLFWDebug();
	void startOpenGLDebug();

	void checkGLError(const char* msg = "");

	static void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam);

	int width_;
	int height_;
	std::string window_name_;
	GLFWwindow* window;
};

}