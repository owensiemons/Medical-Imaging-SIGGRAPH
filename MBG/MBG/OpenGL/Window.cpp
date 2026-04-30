#include "Window.hpp"

namespace MBG {

void APIENTRY Window::glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 8) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void Window::checkGLError(const char* msg)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error (" << msg << "): " << err << std::endl;
	}
}

void Window::startGLFWDebug()
{
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
		});
}

void Window::startOpenGLDebug()
{
#if !defined(__APPLE__)
	/*
	const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR)); // The document CLAIMS this name does not change from release to release, but I'm not sure if the name for MacOS is "Apple"
	const bool b_mac_os = strncmp(vendor, "Apple", strlen(vendor)) == 0;
	if (b_mac_os)
	{
		return;
	}
	assert(!b_mac_os);
	*/
	

	bool b_debug_output_supported = glfwExtensionSupported("GL_ARB_debug_output") == GL_TRUE;

	/*
	int num_extensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
	for (int extension_index = 0; extension_index < num_extensions; ++extension_index)
	{
		const char* extension_string = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, extension_index));
		b_debug_output_supported = strncmp(extension_string, "GL_ARB_debug_output", strlen(extension_string)) == 0; // I am not sure if this code supports AMD GPUs. It might neeed to check for AMD_debug_output.
		if (b_debug_output_supported)
		{
			break;
		}
	}
	*/

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (b_debug_output_supported)
	{
		//std::cerr << "Window::startOpenGLDebug(): This system supports GL_ARB_debug_output." << std::endl;
		
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		// glDebugMessageCallback(glDebugOutput, nullptr);
		PFNGLDEBUGMESSAGECALLBACKARBPROC pfnGlDebugMessageCallbackARB = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKARBPROC>(glfwGetProcAddress("glDebugMessageCallbackARB"));
		if (pfnGlDebugMessageCallbackARB == NULL)
		{
			std::cerr << "Window::startOpenGLDebug(): Failed to load glDebugMessageCallbackARB." << std::endl;
			return;
		}
		PFNGLDEBUGMESSAGECONTROLPROC pfnGlDebugMessageControlARB = reinterpret_cast<PFNGLDEBUGMESSAGECONTROLPROC>(glfwGetProcAddress("glDebugMessageControlARB"));
		if (pfnGlDebugMessageControlARB == NULL)
		{
			std::cerr << "Window::startOpenGLDebug(): Failed to load glDebugMessageControlARB." << std::endl;
			return;
		}
		pfnGlDebugMessageCallbackARB(glDebugOutput, nullptr);
		pfnGlDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		//std::cerr << "Window::startOpenGLDebug(): Completed loading functions for GL_ARB_debug_output." << std::endl;
	}
#endif
}

void Window::framebuffer_size_callback_static(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0) {
		return;
	}
	std::vector<void*>* ptrs = static_cast<std::vector<void*>*>(glfwGetWindowUserPointer(window));
	if (ptrs) {
		Window* instance = static_cast<Window*>((*ptrs)[0]);
		instance->framebuffer_size_callback(window, width, height);
	}
}

// Non-static member function that handles the event
void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	width_ = width;
	height_ = height;
}

// create in window mode
Window::Window(int width, int height, std::string window_name = "Unnamed") : width_(width), height_(height), window_name_(window_name)
{
#ifdef _DEBUG
	startGLFWDebug();
#endif
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(width, height, window_name_.c_str(), NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	//glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback_static);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	startOpenGLDebug();
#endif
}


bool Window::isClosed() const
{
	return glfwWindowShouldClose(window);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

}