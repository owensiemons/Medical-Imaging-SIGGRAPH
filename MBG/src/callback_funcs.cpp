#include "callback_funcs.hpp"
#include <imgui.h>


bool firstMouse = true;
float lastX = static_cast<float>(initWidth) / 2.0;
float lastY = static_cast<float>(initHeight) / 2.0;
bool islmbHeld = false;
float orbitRadius = 2.0;

std::vector<std::string> shader_files = {
	"Shaders/alpha_blender.glsl",
	"Shaders/isosurface.glsl",
	"Shaders/MIP.glsl",
	"Shaders/PBR.glsl"
};

size_t shader_idx = 0;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) return;

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

		shader_idx++;
		render_pass_main->changeShader(shader_files[shader_idx % shader_files.size()]);
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	if (ImGui::GetIO().WantCaptureMouse) return;

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
	if (ImGui::GetIO().WantCaptureMouse) return;

	std::vector<void*>* callback_ptrs = static_cast<std::vector<void*>*>(glfwGetWindowUserPointer(window));
	Camera* camera = static_cast<Camera*>((*callback_ptrs)[0]);
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