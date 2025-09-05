#pragma once

#include "ODGameObject.h"
#include "ODWindow.h"

namespace ODEngine {
    class KeyboardMovementController {
        public:
              struct KeyMappings {
                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_Q;
                int moveDown = GLFW_KEY_E;
                int lookLeft = GLFW_KEY_LEFT;
                int lookRight = GLFW_KEY_RIGHT;
                int lookUp = GLFW_KEY_UP;
                int lookDown = GLFW_KEY_DOWN;
            };
        
        void init_callbacks(GLFWwindow* window);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        
        void HandleInputs(GLFWwindow* window, float dt, ODGameObject& gameObject);
        
        private:
        void MoveInPlaneXZ(GLFWwindow* window, float dt, ODGameObject& gameObject);
        void HandleScrolling(float dt, ODGameObject& gameObject);
        
        private:
            KeyMappings keys{};
            float m_moveSpeed = 2.5f;
            float m_lookSpeed = 1.5f;
            float m_scrollSpeed = 50.0f;

            float m_lastScrollY = 0.0f;
    };
}