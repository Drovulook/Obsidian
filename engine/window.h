#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace od_engine {
    class Window {
        public:
            Window(int width, int height, std::string name);
            ~Window();

            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;

            bool shouldClose() const { return glfwWindowShouldClose(m_window_ptr); }

        private:
            void initWindow();
            
            const int m_width;
            const int m_height;

            std::string m_windowName;

            GLFWwindow* m_window_ptr;
    };
}