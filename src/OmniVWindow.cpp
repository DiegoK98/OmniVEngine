#include "OmniVWindow.hpp"

// std
#include <stdexcept>

namespace OmniV {

	OmniVWindow::OmniVWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	OmniVWindow::~OmniVWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void OmniVWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void OmniVWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to craete window surface");
		}
	}

	void OmniVWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto omnivWindow = reinterpret_cast<OmniVWindow*>(glfwGetWindowUserPointer(window));
		omnivWindow->framebufferResized = true;
		omnivWindow->width = width;
		omnivWindow->height = height;
	}
}