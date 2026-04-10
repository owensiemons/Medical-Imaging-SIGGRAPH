#include "common.hpp"

using namespace glm;
using namespace MBG;

int initWidth = 1600;
int initHeight = 1200;

// TODO: tranfser function include gradient?, add temporal accumulation?, add gui, possibly redo isosurface with marching cubes?, isosurface shadows

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

	// ----------------- SSBOs (transfer function) -----------------------------
	rgb_transfer_elem rgb_transfer_data[3] = {// This should be sorted by the density value
		{vec3(0.5, 0.255, 1.0), 0.0},
		{vec3(0.8, 0.3, 0.5), 0.4},
		{vec3(1.0, 0.65, 0.0), 0.8}
	};
	uint rgb_transfer_data_size = sizeof(rgb_transfer_data) / sizeof(rgb_transfer_data[0]);

	ShaderStorageBufferParams rgb_ssbo_params({
		.data = &rgb_transfer_data,
		.size = sizeof(rgb_transfer_data),
		.buffer_usage = BUFFER_USAGE::DYNAMIC_DRAW,
		.binding = 3,
		});

	ShaderStorageBuffer rgb_ssbo(rgb_ssbo_params);

	a_transfer_elem a_transfer_data[3] = {// This should be sorted by the density value
		{0.0, 0.33},
		{0.12, 0.4},
		{0.0, 0.47}
	};
	uint a_transfer_data_size = sizeof(a_transfer_data) / sizeof(a_transfer_data[0]);

	ShaderStorageBufferParams a_ssbo_params({
		.data = &a_transfer_data,
		.size = sizeof(a_transfer_data),
		.buffer_usage = BUFFER_USAGE::DYNAMIC_DRAW,
		.binding = 4,
		});

	ShaderStorageBuffer a_ssbo(a_ssbo_params);

	// ----------------- Uniforms -----------------------------
	mat4 proj_matrix = camera.getCameraProjMat();
	mat4 view_matrix = camera.getCameraViewMat();

	glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));// only in isosurface

	uint frame_count = 0;
	vec3 aabb_bounds = vec3(sx, sy, sz) - vec3(-sx, -sy, -sz);
	uniforms uniform_data = { vec2((float)window.getWidth(), (float)window.getHeight()),
		frame_count, 0.0, proj_matrix, view_matrix, model_matrix,
		inverse(proj_matrix), inverse(view_matrix),
		vec3(sx, sy, sz), 0.0, vec3(-sx,-sy,-sz), 0.0, rgb_transfer_data_size, a_transfer_data_size,
		vec2(aabb_bounds.x / 2, -aabb_bounds.x / 2), vec2(aabb_bounds.y / 2, -aabb_bounds.y / 2), vec2(aabb_bounds.z / 2, -aabb_bounds.z / 2)
	};

	UniformBufferParams ubo_params({
		.data = &uniform_data,
		.size = sizeof(uniform_data),
		.buffer_usage = BUFFER_USAGE::STATIC_DRAW,
		});

	UniformBuffer ubo(ubo_params);

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
	RenderPass render_pass_main(shader_files[0]);// 0 = alpha blender, 1 = isosurface, 2 = MIP 3 = PBR
	callback_ptrs.push_back(static_cast<void*>(&render_pass_main));

	// ----------------- Descriptor Set -----------------------------
	Descriptor descriptors[] = {
		{DESCRIPTOR_TYPE::VERTEX_BUFFER_IN, (void*)&vertex_buffer, nullptr},
		{DESCRIPTOR_TYPE::UNIFORM_BUFFER, (void*)&ubo, nullptr},
		{DESCRIPTOR_TYPE::TEXTURE_3D_BUFFER_IN, (void*)&volume_texture, nullptr},
		{DESCRIPTOR_TYPE::SHADER_STORAGE_BUFFER, (void*)&rgb_ssbo, nullptr},
		{DESCRIPTOR_TYPE::SHADER_STORAGE_BUFFER, (void*)&a_ssbo, nullptr }
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

	graph.addNodeDisplay(); // display frame

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
			vec3(sx, sy, sz), 0.0, vec3(-sx,-sy,-sz), 0.0,
			rgb_transfer_data_size, a_transfer_data_size,
			vec2(aabb_bounds.x / 2, -aabb_bounds.x / 2), vec2(aabb_bounds.y / 2, -aabb_bounds.y / 2), vec2(aabb_bounds.z / 2, -aabb_bounds.z / 2)
		};

		ubo.remapData((size_t)sizeof(window_data), &window_data, 0);

		frame_count++;
	}
}
