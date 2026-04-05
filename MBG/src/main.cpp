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
#include <glm/ext/matrix_transform.hpp>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);

int initWidth = 1600;
int initHeight = 1200;

bool firstMouse = true;
float lastX = static_cast<float>(initWidth) / 2.0;
float lastY = static_cast<float>(initHeight) / 2.0;
bool islmbHeld = false;
float orbitRadius = 2.0;

uint shader_idx = 0;

std::vector<std::string> shader_files = {
	"Shaders/alpha_blender.glsl",
	"Shaders/isosurface.glsl",
	"Shaders/MIP.glsl",
	"Shaders/PBR.glsl"
};
// TODO: make PBR more PBR, transfer functions, add occlusion plane things, add temporal accumulation?, add gui


int main() {
	// ----------------- Window -----------------------------
	Window window(initWidth, initHeight, "raymarch");
	GLFWwindow* glfw_wind = window.getWindow();

	glfwSetMouseButtonCallback(glfw_wind, mouse_button_callback);
	glfwSetCursorPosCallback(glfw_wind, mouse_callback);
	glfwSetScrollCallback(glfw_wind, scroll_callback);

	std::vector<void*> callback_ptrs;
	glfwSetWindowUserPointer(glfw_wind, &callback_ptrs);
	// ^^ We need to be able to pass the camera, renderpass, etc. objects to the callback functions, we need to use WindowUserPointer for that,
	// my solution is to use a vector of void* and static cast them into the correct objects later

	// ----------------- Camera -----------------------------
	Camera camera(vec3(0.0, 0.0, -2.0), (float)window.getWidth() / (float)window.getHeight());
	callback_ptrs.push_back(static_cast<void*>(&camera));

	// ----------------- Texture Stuff -----------------------------
	uint32_t width, height, depth;
	unsigned char* volume_data = nullptr;//volume_vector.data();
	std::cout << std::filesystem::absolute("Data/ct_scan.raw") << std::endl;
	Load3DTextureBinary("Data/ct_scan.raw", volume_data, width, height, depth);

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

	// scale the bounding box to match the dimensions of the data, no warping

	float vx = 0.5f, vy = 1.0f, vz = 0.5f; // This is the spacing between voxels from the .nii, ex print(scan.header.get_zooms())
	float px = width * vx;
	float py = height * vy;
	float pz = depth * vz;

	float max_phys = max(px, max(py, pz));
	float sx = px / max_phys;
	float sy = py / max_phys;
	float sz = pz / max_phys;

	// ----------------- Uniforms -----------------------------
	mat4 proj_matrix = camera.getCameraProjMat();
	mat4 view_matrix = camera.getCameraViewMat();

	glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));// only in isosurface

	uint frame_count = 0;

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
		float pad1_;// 32 bytes
	};

	uniforms uniform_data = { vec2((float)window.getWidth(), (float)window.getHeight()),
		frame_count, 0.0, proj_matrix, view_matrix, model_matrix,
		inverse(proj_matrix), inverse(view_matrix),
		vec3(sx, sy, sz), 0.0, vec3(-sx,-sy,-sz), 0.0
	};

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
		{vec3(-sx, -sy,  sz)}, {vec3(sx, -sy,  sz)}, {vec3(sx,  sy,  sz)},
		{vec3(-sx, -sy,  sz)}, {vec3(sx,  sy,  sz)}, {vec3(-sx,  sy,  sz)},
		// back
		{vec3(sx, -sy, -sz)}, {vec3(-sx, -sy, -sz)}, {vec3(-sx,  sy, -sz)},
		{vec3(sx, -sy, -sz)}, {vec3(-sx,  sy, -sz)}, {vec3(sx,  sy, -sz)},
		// left
		{vec3(-sx, -sy, -sz)}, {vec3(-sx, -sy,  sz)}, {vec3(-sx,  sy,  sz)},
		{vec3(-sx, -sy, -sz)}, {vec3(-sx,  sy,  sz)}, {vec3(-sx,  sy, -sz)},
		// right
		{vec3(sx, -sy,  sz)}, {vec3(sx, -sy, -sz)}, {vec3(sx,  sy, -sz)},
		{vec3(sx, -sy,  sz)}, {vec3(sx,  sy, -sz)}, {vec3(sx,  sy,  sz)},
		// top
		{vec3(-sx,  sy,  sz)}, {vec3(sx,  sy,  sz)}, {vec3(sx,  sy, -sz)},
		{vec3(-sx,  sy,  sz)}, {vec3(sx,  sy, -sz)}, {vec3(-sx,  sy, -sz)},
		// bottom
		{vec3(-sx, -sy, -sz)}, {vec3(sx, -sy, -sz)}, {vec3(sx, -sy,  sz)},
		{vec3(-sx, -sy, -sz)}, {vec3(sx, -sy,  sz)}, {vec3(-sx, -sy,  sz)},
	};

	VertexBuffer vertex_buffer({
		.attributes = vertex_attributes,
		.attributes_count = sizeof(vertex_attributes) / sizeof(Attributes),
		.data = vertex_data.data(),
		.count = (uint)vertex_data.size(),
		});


	// ----------------- Render Pass -----------------------------
	RenderPass render_pass_main(shader_files[2]);// 0 = alpha blender, 1 = isosurface, 2 = MIP 3 = PBR
	callback_ptrs.push_back(static_cast<void*>(&render_pass_main));

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
		.color = vec4(0.09, 0.09, 0.09, 1.0),
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

		if (fbHeight > 0) {
			camera.updateAspectRatio((float)fbWidth / (float)fbHeight);
		}

		proj_matrix = camera.getCameraProjMat();
		view_matrix = camera.getCameraViewMat();

		uniforms window_data = {
			vec2((float)fbWidth, (float)fbHeight),
			frame_count, 0.0,
			proj_matrix, view_matrix, model_matrix,
			inverse(proj_matrix), inverse(view_matrix),
			vec3(sx, sy, sz), 0.0, vec3(-sx,-sy,-sz), 0.0
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

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		std::vector<void*>* callback_ptrs = static_cast<std::vector<void*>*>(glfwGetWindowUserPointer(window));
		RenderPass* render_pass_main = static_cast<RenderPass*>((*callback_ptrs)[1]);

		render_pass_main->changeShader(shader_files[shader_idx % shader_files.size()]);
		shader_idx++;
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	if (islmbHeld) {
		std::vector<void*>* callback_ptrs = static_cast<std::vector<void*>*>(glfwGetWindowUserPointer(window));
		Camera* camera = static_cast<Camera*>((*callback_ptrs)[0]);

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
		camera->orbit(mouseDelta, target, orbitRadius);
	}
}

void scroll_callback(GLFWwindow* window, double xposIn, double yposIn) {
	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	//float currFOV = camera->getFOV();

	float ypos = static_cast<float>(yposIn);

	float sensitivity = 0.07f;
	ypos *= sensitivity;

	orbitRadius -= ypos;
	if (orbitRadius <= 0.1) {
		orbitRadius = 0.1;
	}

	camera->orbit(vec2(0.0, 0.0), vec3(0.0, 0.0, 0.0), orbitRadius);

}
