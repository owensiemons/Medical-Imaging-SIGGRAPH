#include "common.hpp"
using namespace glm;
using namespace MBG;

int initWidth = 1600;
int initHeight = 1200;

// TODO: tranfser function include gradient?, add temporal accumulation?, add gui, possibly redo isosurface with marching cubes?, isosurface shadows

int main() {
	// ----------------- Window -----------------------------
	std::vector<void*> callback_ptrs;

	Window window(initWidth, initHeight, "Renderer");
	GLFWwindow* glfw_wind = window.getWindow();

	glfwSetMouseButtonCallback(glfw_wind, mouse_button_callback);
	glfwSetCursorPosCallback(glfw_wind, mouse_callback);
	glfwSetScrollCallback(glfw_wind, scroll_callback);

	glfwSetWindowUserPointer(glfw_wind, &callback_ptrs);
	callback_ptrs.push_back(static_cast<void*>(&window));
	// ^^ We need to be able to pass the camera, renderpass, etc. objects to the callback functions, we need to use WindowUserPointer for that,
	// my solution is to use a vector of void* and static cast them into the correct objects later

	// Set Up ImGui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(glfw_wind, true);
	ImGui_ImplOpenGL3_Init("#version 430");


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
	std::list<ImGG::Mark> marks;// Default gradient

	marks.push_back(ImGG::Mark{ ImGG::RelativePosition(0.0), ImGG::ColorRGBA(0.0, 0.0, 0.0, 1.0) });
	marks.push_back(ImGG::Mark{ ImGG::RelativePosition(0.27), ImGG::ColorRGBA(0.42, 0.11, 1.0, 1.0) });
	marks.push_back(ImGG::Mark{ ImGG::RelativePosition(0.46), ImGG::ColorRGBA(0.81, 0.27, 0.49, 1.0) });
	marks.push_back(ImGG::Mark{ ImGG::RelativePosition(0.63), ImGG::ColorRGBA(1.0, 0.65, 0.0, 1.0) });
	marks.push_back(ImGG::Mark{ ImGG::RelativePosition(0.83), ImGG::ColorRGBA(1.0, 0.9, 0.78, 1.0) });

	std::vector<rgb_transfer_elem> rgb_transfer_data;

	for (auto it = marks.begin(); it != marks.end(); ++it) {
		ImGG::ColorRGBA col = (*it).color;
		ImGG::RelativePosition pos = (*it).position;
		rgb_transfer_data.push_back({ vec3(col.x, col.y, col.z), pos.get()});
	}

	ShaderStorageBufferParams rgb_ssbo_params(
		(void*)rgb_transfer_data.data(),
		32 * sizeof(rgb_transfer_elem),// max 32 size
		BUFFER_USAGE::DYNAMIC_DRAW,
		0
	);

	ShaderStorageBuffer rgb_ssbo(rgb_ssbo_params);

	std::vector<a_transfer_elem> a_transfer_data = {// This should be sorted by the density value
		{-0.01, 0.0},
		{0.08, 0.4},
		{0.55, 0.58}
	};

	ShaderStorageBufferParams a_ssbo_params(
        a_transfer_data.data(),
		32 * sizeof(a_transfer_elem),// max 32 size
		BUFFER_USAGE::DYNAMIC_DRAW,
		1
	);

	ShaderStorageBuffer a_ssbo(a_ssbo_params);

	// ----------------- Uniforms -----------------------------
	vec3 bg_color = vec3(0.09, 0.09, 0.09);
	float step_size = 0.03;
	float light_step_size = 0.08;
	float light_pos = 4;
	float spec_power = 9.0;
	float ka = 0.001;
	float kd = 0.9;
	float ks = 0.9;

	float threshold = 0.3;
	float scatter = 0.8;
	float absorption = 0.9;
	float asymmetry = 0.0;

	mat4 proj_matrix = camera.getCameraProjMat();
	mat4 view_matrix = camera.getCameraViewMat();

	glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));// only in isosurface

	uint frame_count = 0;
	vec3 aabb_bounds = vec3(sx, sy, sz) - vec3(-sx, -sy, -sz);

	main_uniforms main_uniform_data = { vec2((float)window.getWidth(), (float)window.getHeight()),
		frame_count, 0.0, proj_matrix, view_matrix, model_matrix,
		inverse(proj_matrix), inverse(view_matrix),
		vec3(sx, sy, sz), 0.0, vec3(-sx,-sy,-sz), 0.0, rgb_transfer_data.size(), a_transfer_data.size(),
		vec2(aabb_bounds.x / 2, -aabb_bounds.x / 2), vec2(aabb_bounds.y / 2, -aabb_bounds.y / 2), vec2(aabb_bounds.z / 2, -aabb_bounds.z / 2),
		bg_color, 0.0, step_size, light_step_size, light_pos, 0.0
	};

	special_uniforms special_uniform_data{
		vec4(ka, kd, ks, spec_power), vec4(threshold, scatter, absorption, asymmetry)
	};

	UniformBufferParams main_ubo_params({
		.data = &main_uniform_data,
		.size = sizeof(main_uniform_data),
		.buffer_usage = BUFFER_USAGE::STATIC_DRAW,
		});

	UniformBufferParams special_ubo_params({
		.data = &special_uniform_data,
		.size = sizeof(special_uniform_data),
		.buffer_usage = BUFFER_USAGE::STATIC_DRAW,
		});

	UniformBuffer main_ubo(main_ubo_params);

	UniformBuffer special_ubo(special_ubo_params);

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
		{DESCRIPTOR_TYPE::UNIFORM_BUFFER, (void*)&main_ubo, nullptr},
		{DESCRIPTOR_TYPE::UNIFORM_BUFFER, (void*)&special_ubo, nullptr},
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
		.color = vec4(bg_color, 1.0),
		});

	// Render the geometry
	graph.addNode({
		.render_pass = &render_pass_main,
		.render_states = {},
		.descriptor_set = &descriptor_set,
		});


	graph.addNodeImGui();
	graph.addNodeDisplay();

	graph.build();


	// gradient lib stuff
	ImGG::GradientWidget rgb_grad(marks);

	ImGG::Settings grad_settings{};

	grad_settings.gradient_width = 650.f;
	//grad_settings.gradient_height = 100.f;
	//grad_settings.horizontal_margin = 10.f;
	grad_settings.flags = ImGG::Flag::NoAddButton | ImGG::Flag::NoRemoveButton | ImGG::Flag::NoBorder;

	float e = 0.001;// Fixes an near infinitely thin plane if data is at bounds of bounding box

	// ----------------- Clip Values -----------------------------
	float sagittal_clip   = sx;
	float frontal_clip    = sy;
	float transverse_clip = sz;

	auto render_imgui = [&]() {

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Controls");
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Background Color");
		ImGui::SameLine();
		ImGui::ColorEdit3("##bg_color", &bg_color[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		ImGui::SliderFloat("Step Size", &step_size, 0.001f, 0.1f);
		if (shader_idx % shader_files.size() == 1 || shader_idx % shader_files.size() == 3) {
			ImGui::SliderFloat("Light Step Size", &light_step_size, 0.001f, 0.1f);
			ImGui::DragFloat("Light Pos", &light_pos, 0.01f);
		}
		ImGui::Separator();

		bool opacityChanged = false;
		bool densityChanged = false;

		if (shader_idx % shader_files.size() == 0) {


			if (ImPlot::BeginPlot("Opacity Function", ImVec2(-1, 0), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText)) {
				ImPlot::SetupAxis(ImAxis_X1, "Density");
				ImPlot::SetupAxisLimits(ImAxis_X1, -0.2, 1.2);

				ImPlot::SetupAxis(ImAxis_Y1, "Opacity");
				ImPlot::SetupAxisLimits(ImAxis_Y1, -0.2, 1.2);

				ImPlotSpec line_spec;
				line_spec.Marker = ImPlotMarker_Circle;
				line_spec.MarkerSize = 5.5;
				line_spec.LineWeight = 2;

				if (ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl && a_transfer_data.size() <= 32) {
					a_transfer_elem new_elem = { (float)glm::clamp(ImPlot::GetPlotMousePos().y, -1.0, 1.0), (float)glm::clamp(ImPlot::GetPlotMousePos().x, 0.0, 1.0)};

					bool found = false;
					for (int i = 0; i < a_transfer_data.size(); i++) {
						if (a_transfer_data[i].dens > new_elem.dens) {
							a_transfer_data.insert(a_transfer_data.begin() + i, new_elem);
							found = true;
							break;
						}
					}
					if (!found) {
						a_transfer_data.push_back(new_elem);
					}
					a_ssbo.remapData(a_transfer_data.size() * sizeof(a_transfer_elem), a_transfer_data.data(), 0);
				}

				for (int i = 0; i < a_transfer_data.size(); i++) {
					double dx = a_transfer_data[i].dens;
					double dy = a_transfer_data[i].opacity;
					if (ImPlot::DragPoint(i, &dx, &dy, ImVec4(0, 0, 0, 1))) {
						double low = (i > 0) ? (double)a_transfer_data[i - 1].dens : 0.0;
						double high = (i < a_transfer_data.size() - 1) ? (double)a_transfer_data[i + 1].dens : 1.0;

						a_transfer_data[i].dens = (float)glm::clamp(dx, low, high);
						a_transfer_data[i].opacity = (float)glm::clamp(dy, -1.0, 1.0);

						densityChanged = true;
						opacityChanged = true;
					}
				}

				ImPlot::PlotLineG("opacity plot", [](int idx, void* data) {
					auto* arr = (a_transfer_elem*)data;
					return ImPlotPoint(arr[idx].dens, arr[idx].opacity);
					}, a_transfer_data.data(), a_transfer_data.size(), line_spec);
				ImPlot::EndPlot();
			}
			if (ImGui::Button("Reset")) {
				a_transfer_data.clear();
			}
			ImGui::Separator();
		}

		rgb_grad.widget("Color Gradient", grad_settings);
		ImGui::Separator();

		rgb_transfer_data.clear();
		for (auto it = rgb_grad.gradient().get_marks().begin(); it != rgb_grad.gradient().get_marks().end(); ++it) {// Yes, we remap every frame, its a pain to do otherwise prob
			ImGG::ColorRGBA col = (*it).color;
			ImGG::RelativePosition pos = (*it).position;
			rgb_transfer_data.push_back({ vec3(col.x, col.y, col.z), pos.get() });
		}

		rgb_ssbo.remapData(rgb_transfer_data.size() * sizeof(rgb_transfer_elem), rgb_transfer_data.data(), 0);



		if (shader_idx % shader_files.size() == 3) {
			ImGui::SliderFloat("Scattering", &scatter, 0.0f, 1.0f);
			ImGui::SliderFloat("Absorption", &absorption, 0.0f, 1.0f);
			ImGui::SliderFloat("Asymmetry", &asymmetry, -1.0f, 1.0f);
				ImGui::Separator();
		}

		if (shader_idx % shader_files.size() == 1) {
			ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);
			ImGui::SliderFloat("K_a", &ka, 0.0f, 1.0f);
			ImGui::SliderFloat("K_d", &kd, 0.0f, 4.0f);
			ImGui::SliderFloat("K_s", &ks, 0.0f, 4.0f);
			ImGui::SliderFloat("Spec. Power", &spec_power, 0.0f, 16.0f);
			ImGui::Separator();
		}

		if (densityChanged || opacityChanged) {
			a_ssbo.remapData(a_transfer_data.size() * sizeof(a_transfer_elem), a_transfer_data.data(), 0);
		}


		// Sliders
		ImGui::SliderFloat("Sagittal", &sagittal_clip, -sx - e, sx + e);
		ImGui::SliderFloat("Frontal", &frontal_clip, -sy - e, sy + e);
		ImGui::SliderFloat("Transverse", &transverse_clip, -sz - e, sz + e);

		if (ImGui::Button("Next Shader")) {
			shader_idx++;
			render_pass_main.changeShader(shader_files[shader_idx % shader_files.size()]);
		}

		ImGui::SameLine();
		ImGui::Separator();
		ImGui::Text("%s", shader_files[shader_idx % shader_files.size()].c_str());
		ImGui::End();
		ImGui::Render();
		};// makes code cleaner
	while (!window.isClosed()) {

		if (glfwWindowShouldClose(glfw_wind)) {
			break;
		}

		if (glfwGetWindowAttrib(glfw_wind, GLFW_ICONIFIED)) {
			glfwWaitEvents();
			continue;
		}

		int fbWidth, fbHeight;
		glfwGetFramebufferSize(glfw_wind, &fbWidth, &fbHeight);

		// --- ImGui widgets ---
		render_imgui();

		// renew framegraph
		graph.addNode({
		.color = vec4(bg_color, 1.0),
			});

		graph.addNode({
			.render_pass = &render_pass_main,
			.render_states = {},
			.descriptor_set = &descriptor_set,
			});

		graph.addNodeImGui();
		graph.addNodeDisplay();
		graph.build();
		graph.run();

		// update uniforms
		if (fbHeight > 0) {
			camera.updateAspectRatio((float)fbWidth / (float)fbHeight);
		}

		proj_matrix = camera.getCameraProjMat();
		view_matrix = camera.getCameraViewMat();

		main_uniforms update_main_ubo = {
			vec2((float)fbWidth, (float)fbHeight),
			frame_count, 0.0,
			proj_matrix, view_matrix, model_matrix,
			inverse(proj_matrix), inverse(view_matrix),
			vec3(sx, sy, sz), 0.0, vec3(-sx,-sy,-sz), 0.0,
			rgb_transfer_data.size(), a_transfer_data.size(),
			vec2(sagittal_clip, -sx), vec2(frontal_clip, -sy), vec2(transverse_clip, -sz),
			bg_color, 0.0, step_size, light_step_size, light_pos, 0.0
		};

		special_uniforms update_special_ubo = {
			vec4(ka, kd, ks, spec_power), vec4(threshold, scatter, absorption, asymmetry)
		};

		main_ubo.remapData((size_t)sizeof(update_main_ubo), &update_main_ubo, 0);

		special_ubo.remapData((size_t)sizeof(update_special_ubo), &update_special_ubo, 0);

		frame_count++;
	
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	return 0;
}
