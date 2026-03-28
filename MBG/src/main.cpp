#include <glm/glm.hpp>
using namespace glm;

#include "../MBG/OpenGL/MBG.hpp"
using namespace MBG;

#include "camera.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>
#include <vector>
#include "texgen.hpp"
#include <filesystem>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);

int initWidth = 1600;
int initHeight = 1200;

bool firstMouse = true;
float lastX = static_cast<float>(initWidth) / 2.0;
float lastY = static_cast<float>(initHeight) / 2.0;
bool islmbHeld = false;

//messing around with a bounding box and some more discrete data

//TODO: fix orbit radius?, isosurface & MIP shaders, make PBR more PBR


int main() {
	// ----------------- Window -----------------------------
	Window window(initWidth, initHeight, "raymarch");
	GLFWwindow* glfw_wind = window.getWindow();

	glfwSetMouseButtonCallback(glfw_wind, mouse_button_callback);
	glfwSetCursorPosCallback(glfw_wind, mouse_callback);
	glfwSetScrollCallback(glfw_wind, scroll_callback);

	// ----------------- Camera -----------------------------
	Camera camera(vec3(0.0, 0.0, -2.0), (float)window.getWidth() / (float)window.getHeight());
	glfwSetWindowUserPointer(glfw_wind, &camera);

	// ----------------- Uniforms -----------------------------
	mat4 proj_matrix = camera.getCameraProjMat();
	mat4 view_matrix = camera.getCameraViewMat();

	mat4 model_matrix = mat4(1.0f);

	uint frame_count = 0;

	struct uniforms {
		float screen_width;
		float screen_height;
		float pad0, pad1;

		mat4 proj;
		mat4 view;
		mat4 model;

		mat4 inv_proj;
		mat4 inv_view;

		uint frame_cnt;
	};

	uniforms uniform_data = { (float)window.getWidth(), (float)window.getHeight(), 0.0f, 0.0f, proj_matrix, view_matrix, model_matrix, inverse(proj_matrix), inverse(view_matrix), frame_count};

	UniformBufferParams params({
		.data = &uniform_data,
		.size = sizeof(uniform_data),
		.buffer_usage = BUFFER_USAGE::STATIC_DRAW,
		});

	UniformBuffer ubo(params);

	// ----------------- Vertex Buffer -----------------------------
	struct Vertex {
		vec3 position;
	};

	Attributes vertex_attributes[] = {
		{ATTRIBUTE::FLOAT, 3},	// postion
	};

	std::vector<Vertex> vertex_data = {
		// front
		{vec3(-1.0, -1.0,  1.0)}, {vec3(1.0, -1.0,  1.0)}, {vec3(1.0,  1.0,  1.0)},
		{vec3(-1.0, -1.0,  1.0)}, {vec3(1.0,  1.0,  1.0)}, {vec3(-1.0,  1.0,  1.0)},
		// back
		{vec3(1.0, -1.0, -1.0)}, {vec3(-1.0, -1.0, -1.0)}, {vec3(-1.0,  1.0, -1.0)},
		{vec3(1.0, -1.0, -1.0)}, {vec3(-1.0,  1.0, -1.0)}, {vec3(1.0,  1.0, -1.0)},
		// left
		{vec3(-1.0, -1.0, -1.0)}, {vec3(-1.0, -1.0,  1.0)}, {vec3(-1.0,  1.0,  1.0)},
		{vec3(-1.0, -1.0, -1.0)}, {vec3(-1.0,  1.0,  1.0)}, {vec3(-1.0,  1.0, -1.0)},
		// right
		{vec3(1.0, -1.0,  1.0)}, {vec3(1.0, -1.0, -1.0)}, {vec3(1.0,  1.0, -1.0)},
		{vec3(1.0, -1.0,  1.0)}, {vec3(1.0,  1.0, -1.0)}, {vec3(1.0,  1.0,  1.0)},
		// top
		{vec3(-1.0,  1.0,  1.0)}, {vec3(1.0,  1.0,  1.0)}, {vec3(1.0,  1.0, -1.0)},
		{vec3(-1.0,  1.0,  1.0)}, {vec3(1.0,  1.0, -1.0)}, {vec3(-1.0,  1.0, -1.0)},
		// bottom
		{vec3(-1.0, -1.0, -1.0)}, {vec3(1.0, -1.0, -1.0)}, {vec3(1.0, -1.0,  1.0)},
		{vec3(-1.0, -1.0, -1.0)}, {vec3(1.0, -1.0,  1.0)}, {vec3(-1.0, -1.0,  1.0)},
	};

	VertexBuffer vertex_buffer({
		.attributes = vertex_attributes,
		.attributes_count = sizeof(vertex_attributes) / sizeof(Attributes),
		.data = vertex_data.data(),
		.count = (uint)vertex_data.size(),
		});



	// ----------------- Render Pass -----------------------------
	//RenderPass render_pass_main("Shaders/alpha_blender.glsl");
	RenderPass render_pass_main("Shaders/PBR.glsl");

	// ----------------- Texture Stuff -----------------------------
	
	uint32_t width, height;
	int depth;
	std::vector<unsigned char> volume_vector = Load3DTexture("C:/Users/rowan/Documents/Graphics/data/8bit", width, height, depth);//Replace with your own
	unsigned char* volume_data = volume_vector.data();

	Texture3DBuffer volume_texture({
		.size = uvec3(width, height, depth),
		.format = TEXTURE_TYPE::R8,
		.min_filter = TEXTURE_FILTER::LINEAR,
		.mag_filter = TEXTURE_FILTER::LINEAR,
		.wrap_s = TEXTURE_WRAP::CLAMP_TO_EDGE,
		.wrap_t = TEXTURE_WRAP::CLAMP_TO_EDGE,
		.wrap_r = TEXTURE_WRAP::CLAMP_TO_EDGE,
		.data = volume_data,
	});
	
	// ----------------- Descriptor Set -----------------------------
	Descriptor descriptors[] = {
		{DESCRIPTOR_TYPE::VERTEX_BUFFER_IN, (void*)&vertex_buffer, nullptr},
		{DESCRIPTOR_TYPE::UNIFORM_BUFFER, (void*)&ubo, nullptr},
		{DESCRIPTOR_TYPE::TEXTURE_3D_BUFFER_IN, (void*)&volume_texture, nullptr}
	};

	DescriptorSetBuffer descriptor_set({
		.descriptors = descriptors,
		.count = sizeof(descriptors) / sizeof(Descriptor),
		});



	// ----------------- Construct Frame Graph -----------------------------
	FrameGraph graph(window);

	graph.addNode({
		.color = vec4(0.0, 0.0, 0.0, 1.0),
		});

	// Render the geometry
	graph.addNode({
		.render_pass = &render_pass_main,
		.render_states = {},
		.descriptor_set = &descriptor_set,
		});

	graph.addNodeDisplay(); // Finally we display the frame

	graph.build();

	while (!window.isClosed()) {
		graph.run();

		int fbWidth, fbHeight;
		glfwGetFramebufferSize(glfw_wind, &fbWidth, &fbHeight);

		camera.updateAspectRatio((float)fbWidth / (float)fbHeight);

		proj_matrix = camera.getCameraProjMat();
		view_matrix = camera.getCameraViewMat();

		uniforms window_data = {
			(float)fbWidth, (float)fbHeight, 0.0f, 0.0f,
			proj_matrix, view_matrix, model_matrix,
			inverse(proj_matrix), inverse(view_matrix),
			frame_count
		};
		
		ubo.remapData((size_t)sizeof(window_data), &window_data, 0);

		frame_count++;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			islmbHeld = true;
			firstMouse = true;
		}
		else if (action == GLFW_RELEASE) {
			islmbHeld = false;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	if (islmbHeld) {
		Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;

			firstMouse = false;
		}

		float dx = lastX - xpos;
		float dy = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.007f;
		dx *= sensitivity;
		dy *= sensitivity;

		if (abs(dx) < 0.001 && abs(dy) < 0.001) {
			return;
		}

		vec2 mouseDelta = vec2(dx, dy);
		vec3 target = vec3(0.0, 0.0, 0.0);
		camera->orbit(mouseDelta, target, 2.0);
	}
}

void scroll_callback(GLFWwindow* window, double xposIn, double yposIn) {
	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	float currFOV = camera->getFOV();

	float ypos = static_cast<float>(yposIn);

	float sensitivity = 5.00f;
	ypos *= sensitivity;

	currFOV -= ypos;
	if (currFOV < 0.5) {
		currFOV = 0.5;
	}

	if (currFOV > 179.5) {
		currFOV = 179.5;
	}
	camera->updateFOV(currFOV);
}
