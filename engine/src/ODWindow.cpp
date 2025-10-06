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

        m_windowPtr = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_windowPtr, this);
        glfwSetFramebufferSizeCallback(m_windowPtr, framebufferResizeCallback);

        if (m_windowPtr) {
             int fbw = 0, fbh = 0;
             glfwGetFramebufferSize(m_windowPtr, &fbw, &fbh);
             if (fbw > 0 && fbh > 0) {
                 m_width = fbw;
                 m_height = fbh;
             }
         }
    }

    ODWindow::~ODWindow() {
        if (m_windowPtr) {
            glfwDestroyWindow(m_windowPtr);
        }
        glfwTerminate();
    }

    VkExtent2D ODWindow::getExtent() {
        if (m_windowPtr) {
            int width = 0, height = 0;
            glfwGetFramebufferSize(m_windowPtr, &width, &height);
            m_width = width;
            m_height = height;
        }
        return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };
    }

    void ODWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
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