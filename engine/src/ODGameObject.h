#pragma once

#include "ODModel.h"

// std
#include <memory>

namespace ODEngine {

    struct Transform2DComponent {
        glm::vec2 translation {};
        glm::vec2 scale {1.0f, 1.0f};
        float rotation;

        glm::mat2 mat2d() { 
            const float s  = glm::sin(rotation);
            const float c  = glm::cos(rotation);
            glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};
            glm::mat2 rotMat{{c, s}, {-s, c}};
            return rotMat * scaleMat; 
        }
    };

    class ODGameObject {
        
        public:
        using id_t = unsigned int; 

        static ODGameObject createGameObject(){
            static id_t currentId = 0;
            return ODGameObject(currentId++);
        };

        ODGameObject(const ODGameObject&) = delete;
        ODGameObject& operator=(const ODGameObject&) = delete; 
        ODGameObject(ODGameObject&&) = default;
        ODGameObject& operator=(ODGameObject&&) = default;

        const id_t getId() const { return m_id; }

        std::shared_ptr<ODModel> model {}; 
        glm::vec3 color {}; 
        Transform2DComponent transform2D{};

        private:
            ODGameObject(id_t id) : m_id(id) {}
            id_t m_id;
    };

}