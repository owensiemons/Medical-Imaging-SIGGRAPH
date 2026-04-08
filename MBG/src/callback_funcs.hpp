#pragma once
#include "common.hpp"

using namespace glm;
using namespace MBG;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xpos, double ypos);