#include "Window.hpp"

// std
#include <stdexcept>

namespace OmniV {

	Window::Window(int w, int h, std::string name) : m_width{ w }, m_height{ h }, m_windowName{ name } {
		initWindow();
	}

	Window::~Window() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Window::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to craete window surface");
		}
	}

	void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height) {
		auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->m_framebufferResized = true;
		window->m_width = width;
		window->m_height = height;
	}
}