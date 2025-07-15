#include "window.h"

namespace od_engine {
    Window::Window(int width, int height, std::string name)
        : m_width(width), m_height(height), m_windowName(std::move(name)){
        initWindow();
    }

    Window::~Window() {
        if (m_window_ptr) {
            glfwDestroyWindow(m_window_ptr);
        }
        glfwTerminate();
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window_ptr = glfwCreateWindow(800, 600, m_windowName.c_str(), nullptr, nullptr);
    }
}