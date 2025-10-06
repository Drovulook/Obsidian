#include "keyboardMovementController.h"
#include <limits>

namespace ODEngine{
    void KeyboardMovementController::init_callbacks(GLFWwindow *window){
        glfwSetWindowUserPointer(window, this);
        glfwSetScrollCallback(window, scroll_callback);
    }

    void KeyboardMovementController::scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
        auto* controller = static_cast<KeyboardMovementController*>(glfwGetWindowUserPointer(window));
        if (controller) {
            controller->m_lastScrollY = static_cast<float>(yoffset);
        }
    }

    void KeyboardMovementController::HandleInputs(GLFWwindow *window, float dt, ODGameObject &gameObject){
        MoveInPlaneXZ(window, dt, gameObject);
        HandleScrolling(dt, gameObject);
    }

    void KeyboardMovementController::MoveInPlaneXZ(GLFWwindow *window, float dt, ODGameObject &gameObject){

        glm::vec3 rotate{0};
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

        if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += m_lookSpeed * dt * glm::normalize(rotate);
        }

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{glm::sin(yaw), 0.0f, glm::cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += m_moveSpeed * dt * glm::normalize(moveDir);
        }

    }
    void KeyboardMovementController::HandleScrolling(float dt, ODGameObject &gameObject){
        if(abs(m_lastScrollY) > std::numeric_limits<float>::epsilon()) {
            assert(gameObject.camera != nullptr && "Object is not a camera");
            float new_fov_value = gameObject.camera->m_perspData.fovy -= m_lastScrollY * dt * m_scrollSpeed;
            new_fov_value = glm::clamp(new_fov_value, glm::radians(25.0f), glm::radians(90.0f));
            gameObject.camera->setPerspectiveProjection(
                new_fov_value,
                gameObject.camera->m_perspData.aspect,
                gameObject.camera->m_perspData.near,
                gameObject.camera->m_perspData.far
            );
            m_lastScrollY = 0.0f;
        }
    }
}