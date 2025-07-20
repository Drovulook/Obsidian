#include "ODWindow.h"

#include <stdexcept>

namespace ODEngine {
    ODWindow::ODWindow(int width, int height, std::string name)
        : m_width(width), m_height(height), m_windowName(std::move(name)){
        initWindow();
    }

    void ODWindow::initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_windowPtr = glfwCreateWindow(800, 600, m_windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_windowPtr, this);
        glfwSetFramebufferSizeCallback(m_windowPtr, framebufferResizeCallback);
    }

    ODWindow::~ODWindow() {
        if (m_windowPtr) {
            glfwDestroyWindow(m_windowPtr);
        }
        glfwTerminate();
    }

    void ODWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface){
        if (glfwCreateWindowSurface(instance, m_windowPtr, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create ODWindow surface!");
        }
    }

    void ODWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height){
        auto odWindow = reinterpret_cast<ODWindow *>(glfwGetWindowUserPointer(window));
        odWindow->m_frameBufferResized = true;
        odWindow->m_width = width;
        odWindow->m_height = height;
    }

}