#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#include <glm/glm.hpp>

namespace ODEngine {
    class ODCamera {
        struct OrthographicData {
            float left;
            float right;
            float bottom;
            float top;
            float near;
            float far;
        };
        struct PerspectiveData {
            float fovy;
            float aspect;
            float near;
            float far;
        };

    public:
        void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
        void setPerspectiveProjection(float fovy, float aspect, float near, float far);
    
        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
        void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4& getProjection() const { return m_projectionMatrix; }
        const glm::mat4& getView() const { return m_viewMatrix; }
        const glm::mat4& getInverseView() const { return m_inverseViewMatrix; }
        glm::vec3 getPosition() const { return glm::vec3(m_inverseViewMatrix[3]); }
        
        void updateOrthographicProjection();
        void updatePerspectiveProjection(float aspect);

    public:
        OrthographicData m_orthoData{};
        PerspectiveData m_perspData{};

    private:
        glm::mat4 m_projectionMatrix{1.0f};
        glm::mat4 m_viewMatrix{1.0f};
        glm::mat4 m_inverseViewMatrix{1.0f};

        bool m_is_perspective = true;
    };
}