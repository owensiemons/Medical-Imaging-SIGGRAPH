#pragma once

#include "../MBG/OpenGL/Window.hpp"
#include <glm/glm.hpp>

#include "camera.hpp"
#include "texgen.hpp"
#include "callback_funcs.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>
#include <vector>
#include <filesystem>
#include <glm/ext/matrix_transform.hpp>
#include <algorithm>
#include <iterator>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace glm;
using namespace MBG;

extern std::vector<std::string> shader_files;
extern size_t shader_idx;
extern int initWidth;
extern int initHeight;

struct uniforms {
	vec2 screen_size;
	uint frame_cnt;
	float pad_;// 16 bytes

	mat4 proj;
	mat4 view;
	mat4 model;// 192 bytes

	mat4 inv_proj;
	mat4 inv_view;// 128 bytes

	vec3 aabb_max;
	float pad0_;
	vec3 aabb_min;
	float pad1_; // 32 bytes

	uint rgb_transfer_arr_size;
	uint a_tranfer_arr_size;
	vec2 x_bounds;// 16 bytes

	vec2 y_bounds;
	vec2 z_bounds;// 16 bytes

	vec3 bg_color;
	float pad2_;// 16 bytes

	float step_size;
	float light_size;
	vec2 pad3_;// 16 bytes
};

struct rgb_transfer_elem {
	vec3 col;
	float dens;// 16 bytes
};

struct a_transfer_elem {
	float opacity;
	float dens;
};