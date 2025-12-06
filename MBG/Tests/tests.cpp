#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
using namespace glm;

#include "../MBG/OpenGL/MBG.hpp"
using namespace MBG;

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>
#include <vector>

TEST_CASE("Simple Triangle") {
	// ----------------- Window -----------------------------
	Window window(800, 600, "Triangle");



	// ----------------- Vertex Buffer -----------------------------
	struct Vertex {
		vec3 position;
		vec3 color;
	};

	Attributes vertex_attributes[] = {
		{ATTRIBUTE::FLOAT, 3},	// postion
		{ATTRIBUTE::FLOAT, 3},	// color
	};

	std::vector<Vertex> vertex_data = {
		{vec3(-0.5, -0.5,  0.0), vec3(1.0, 0.0, 0.0)},
		{vec3( 0.5, -0.5,  0.0), vec3(0.0, 1.0, 0.0)},
		{vec3( 0.0,  0.5,  0.0), vec3(0.0, 0.0, 1.0)}
	};

	VertexBuffer vertex_buffer({
		.attributes = vertex_attributes,
		.attributes_count = sizeof(vertex_attributes) / sizeof(Attributes),
		.data = vertex_data.data(),
		.count = (uint)vertex_data.size(),
	});



	// ----------------- Render Pass -----------------------------
	RenderPass render_pass_main("Shaders/simple_triangle.glsl");



	// ----------------- Descriptor Set -----------------------------
	Descriptor descriptors[] = {
		{DESCRIPTOR_TYPE::VERTEX_BUFFER_IN, (void*)&vertex_buffer, nullptr},
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
		graph.run();
	}
}

TEST_CASE("A nice image") {
	// ----------------- Window -----------------------------
	Window window(800, 600, "My Image");



	// ----------------- Vertex Buffer -----------------------------
	struct Vertex {
		vec3 position;
		vec2 uv;
	};

	Attributes vertex_attributes[] = {
		{ATTRIBUTE::FLOAT, 3},	// postion
		{ATTRIBUTE::FLOAT, 2},	// uv
	};

	std::vector<Vertex> vertex_data = {
		{ vec3(-1.0, -1.0,  0.0), vec2(0.0, 1.0) },
		{ vec3( 1.0, -1.0,  0.0), vec2(1.0, 1.0) },
		{ vec3( 1.0,  1.0,  0.0), vec2(1.0, 0.0) },

		{ vec3(-1.0, -1.0,  0.0), vec2(0.0, 1.0) },
		{ vec3(-1.0,  1.0,  0.0), vec2(0.0, 0.0) },
		{ vec3( 1.0,  1.0,  0.0), vec2(1.0, 0.0) },
	};

	VertexBuffer vertex_buffer({
		.attributes = vertex_attributes,
		.attributes_count = sizeof(vertex_attributes) / sizeof(Attributes),
		.data = vertex_data.data(),
		.count = (uint)vertex_data.size(),
		});



	// ----------------- Render Pass -----------------------------
	RenderPass render_pass_main("Shaders/simple_image.glsl");



	// ----------------- Texture 2D -----------------------------
	
	// load texture from "Assets/MoreClouds.png"
	int width, height, channels;
	stbi_uc* data = stbi_load("Assets/MoreClouds.png", &width, &height, &channels, 4);
	if (!data) {
		std::cerr << "Failed to load texture: Assets/MoreClouds.png\n";
		return;
	}

	Texture2DBuffer my_texture({
		.size = uvec2(width, height),
		.format = TEXTURE_TYPE::RGBA8,
		.min_filter = TEXTURE_FILTER::NEAREST,
		.mag_filter = TEXTURE_FILTER::NEAREST,
		.data = data,
	});

	stbi_image_free(data);



	// ----------------- Descriptor Set -----------------------------
	Descriptor descriptors[] = {
		{DESCRIPTOR_TYPE::VERTEX_BUFFER_IN, (void*)&vertex_buffer, nullptr},
		{DESCRIPTOR_TYPE::TEXTURE_2D_BUFFER_IN, (void*)&my_texture, nullptr}
	};

	DescriptorSetBuffer descriptor_set({
		.descriptors = descriptors,
		.count = sizeof(descriptors) / sizeof(Descriptor),
		});



	// ----------------- Construct Frame Graph -----------------------------
	FrameGraph graph(window);

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
	}
}

float randf() {
	return float(rand() % RAND_MAX) / float(RAND_MAX);
}

// Quadratic kernal
const float k_size = 0.08f;
static float G(vec2 a, vec2 b) {
	vec2 r = a - b;
	float dist2 = dot(r, r);
	float h2 = k_size * k_size;

	if (dist2 >= h2) return 0.0;

	float q = 1.0 - dist2 / h2;
	return q * q;
}

vec2 dG(vec2 a, vec2 b) {
	vec2 r = a - b;
	float dist2 = dot(r, r);
	float h2 = k_size * k_size;

	if (dist2 >= h2) return vec2(0.0);

	float q = 1.0 - dist2 / h2;

	return -4.0f * q * r / h2;
}

TEST_CASE("A simple fluid simulation") {
	// ----------------- Particle Data -----------------------------
	const float epsilon = 1e-5;
	const uint particle_cout = 200;
	const uint sim_itters = 2; // number of itterations per sub-step
	const float rest_density = 1.0f;
	const float dt = 0.01f;
	
	struct Particle {
		vec2 position;
		vec2 last_position;
		vec2 velocity;
		float C;
		float lambda;
	};

	std::array<Particle, particle_cout> particle_data;
	for (auto& v : particle_data) {
		v.position.x = randf() * 2.0 - 1.0;
		v.position.y = randf() * 2.0 - 1.0;
		
		v.last_position = v.position;

		float max_vel = 0.5;
		v.velocity.x = (randf() * 2.0 - 1.0) * max_vel;
		v.velocity.y = (randf() * 2.0 - 1.0) * max_vel;

		v.C = 0.0;
		v.lambda = 0.0;
	}

	Window window(800, 600, "My Fluid");

	Attributes vertex_attributes[] = {
		{ATTRIBUTE::FLOAT, 2},	// position
		{ATTRIBUTE::FLOAT, 2},	// last position
		{ATTRIBUTE::FLOAT, 2},	// velocity position

		{ATTRIBUTE::FLOAT, 1},	// C
		{ATTRIBUTE::FLOAT, 1},	// lambda

	};

	VertexBuffer vertex_buffer({
		.attributes = vertex_attributes,
		.attributes_count = sizeof(vertex_attributes) / sizeof(Attributes),
		.data = particle_data.data(),
		.count = (uint)particle_data.size(),
	});

	RenderPass render_pass_main("Shaders/simple_fluid.glsl");

	Descriptor descriptors[] = {
		{DESCRIPTOR_TYPE::VERTEX_BUFFER_IN, (void*)&vertex_buffer, nullptr}
	};

	DescriptorSetBuffer descriptor_set({
		.descriptors = descriptors,
		.count = sizeof(descriptors) / sizeof(Descriptor),
	});

	FrameGraph graph(window);

	graph.addNode(NodeVertexCopy{
		.vertex_buffer = &vertex_buffer,
		.size = particle_cout,
		.data  = &particle_data,
	});

	graph.addNode({
		.color = vec4(0.0, 0.0, 0.0, 1.0),
	});

	// Render the geometry
	glEnable(GL_PROGRAM_POINT_SIZE); // TODO: We need to add a render mode for this!
	graph.addNode({
		.render_pass = &render_pass_main,
		.render_states = {
			.render_type = RENDER_TYPE::POINTS,
		},
		.descriptor_set = &descriptor_set,
		});

	graph.addNodeDisplay(); // Finally we display the frame

	graph.build();
	
	vec2 mouse_pos = vec2(0.0);

	while (!window.isClosed()) {

		// Get mouse interaction
		if (glfwGetMouseButton(window.getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

			mouse_pos.x = (mouseX / window.getWidth()) * 2.0f - 1.0f;
			mouse_pos.y = (mouseY / window.getHeight()) * 2.0f - 1.0f;

			mouse_pos.y = -mouse_pos.y;
		}

		for (auto& p : particle_data) {
			// apply forces
			p.velocity -= normalize(p.position) * 9.81f * dt; // apply gravity (m/s^2)

			// predict position
			p.position += p.velocity * dt;

			// initilize data
			p.C = 0.0f;
			p.lambda = 0.0f;
		}

		for (unsigned i = 0; i < sim_itters; i++) {
			
			// Get density
			for (auto& p : particle_data) {
				float density = 0.0f;
				for (auto& p_ : particle_data) {
					density += G(p.position, p_.position);
				}
				p.C = density / rest_density - 1.0f; // Eq (1)
			}

			// Get lamdba
			for (auto& p : particle_data) {
				float sumGrad2 = 0.0f;
				vec2 gradCi = vec2(0.0);

				for (auto& p_ : particle_data) {
					if (&p == &p_) continue;

					vec2 dir = p.position - p_.position;
					vec2 grad = (1.0f / rest_density) * dG(p.position, p_.position);

					gradCi += grad;
					sumGrad2 += dot(grad, grad);
				}
				
				sumGrad2 += dot(gradCi, gradCi); // Now we get the special case of C

				p.lambda = -p.C / (sumGrad2 + epsilon);
			}

			// Apply all constraints
			for (auto& p : particle_data) {

				// First we apply the density constraint
				for (auto& p_ : particle_data) {
					if (&p == &p_) continue;

					// Eq (12)
					p.position += (1.0f / rest_density) * (p.lambda + p_.lambda) * dG(p.position, p_.position);
				}

				// Second we apply the bounding box constraint
				p.position = clamp(p.position, vec2(-1.0), vec2(1.0));

				// Third we apply the mouse position as a rigid sphere constraint
				vec2 diff = p.position - mouse_pos;
				float dist = length(diff);

				if (dist < 0.1) {
					vec2 n = diff / dist;
					p.position = mouse_pos + n * 0.1f;
				}
			}
		}

		for (auto& p : particle_data) {
			// update velocity
			p.velocity = (p.position - p.last_position) / dt;

			// update postion
			p.last_position = p.position;
		}

		graph.run();
	}
}