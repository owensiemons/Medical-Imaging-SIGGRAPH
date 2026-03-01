#include <glm/glm.hpp>
using namespace glm;

#include "../MBG/OpenGL/MBG.hpp"
using namespace MBG;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>
#include <vector>
#include "texgen.hpp"
#include <filesystem>
//messing around with a bounding box and some more discrete data

int main() {
	// ----------------- Window -----------------------------
	Window window(800, 600, "raymarch");

	// ----------------- Uniforms -----------------------------
	struct uniforms {
		float screen_width;
		float screen_height;
	};

	uniforms uniform_data = { (float)window.getWidth(), (float)window.getHeight() };

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
		{vec3(-1.0, -1.0, 0.0)},
		{vec3(1.0, -1.0, 0.0)},
		{vec3(1.0, 1.0, 0.0)},

		{vec3(-1.0, 1.0, 0.0)},
		{vec3(-1.0, -1.0, 0.0)},
		{vec3(1.0, 1.0, 0.0)}
	};

	VertexBuffer vertex_buffer({
		.attributes = vertex_attributes,
		.attributes_count = sizeof(vertex_attributes) / sizeof(Attributes),
		.data = vertex_data.data(),
		.count = (uint)vertex_data.size(),
		});



	// ----------------- Render Pass -----------------------------
	RenderPass render_pass_main("Shaders/simple_raymarch.glsl");


	// ----------------- Texture Stuff -----------------------------
	
	uint32_t width, height;
	int depth;
	std::vector<unsigned char> volume_vector = Load3DTexture("C:/Users/rowan/Documents/Graphics/data/8bit", width, height, depth);//Replace with your own
	unsigned char* volume_data = volume_vector.data();
	std::cout << "Width: " << width << " Height: " << height << " Depth: " << depth << std::endl;
	std::cout << "Data size: " << volume_vector.size() << std::endl;
	std::cout << "Data ptr: " << (void*)volume_vector.data() << std::endl;
	Texture3DBuffer volume_texture({
		.size = uvec3(width, height, depth),
		.format = TEXTURE_TYPE::R8,
		.min_filter = TEXTURE_FILTER::NEAREST,
		.mag_filter = TEXTURE_FILTER::NEAREST,
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
		.color = vec4(0.05, 0.06, 0.05, 1.0),
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
		//TODO: update screen resolution (may need to make a new uniformbuffer.hpp function, its a pain).
		graph.run();
	}
}
