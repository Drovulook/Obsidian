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
            VkExtent2D getExtent() { return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}; }

            void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
            
        private:
            void initWindow();
            
            const int m_width;
            const int m_height;

            std::string m_windowName;

            GLFWwindow* m_windowPtr;
    };
}