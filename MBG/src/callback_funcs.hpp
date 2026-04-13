#pragma once
#include "common.hpp"

using namespace glm;
using namespace MBG;

extern std::vector<std::string> shader_files;
extern size_t shader_idx;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xpos, double ypos);