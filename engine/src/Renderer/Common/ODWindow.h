#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ODEngine {
    class ODWindow {
        
        public:
            ODWindow(int width, int height, std::string name);
            ~ODWindow();

            ODWindow(const ODWindow&) = delete;
            ODWindow& operator=(const ODWindow&) = delete;

            bool shouldClose() const { return glfwWindowShouldClose(m_windowPtr); }
            VkExtent2D getExtent();
            bool wasWindowResized() { return m_frameBufferResized; }
            void resetWindowResizedFlag() { m_frameBufferResized = false; }
            GLFWwindow* getGLFWWindow() const { return m_windowPtr; }

            void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
            
        private:
            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            void initWindow();
        
            private:
            int m_width;
            int m_height;
            bool m_frameBufferResized = false;

            std::string m_windowName;

            GLFWwindow* m_windowPtr;
    };
}