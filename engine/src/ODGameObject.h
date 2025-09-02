#pragma once

#include "ODModel.h"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace ODEngine {

    struct TransformComponent {
        glm::vec3 translation {};
        glm::vec3 scale {1.0f, 1.0f, 1.0f};
        glm::vec3 rotation;

        glm::mat4 mat4();
        glm::mat3 normalMatrix();

    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };

    class ODGameObject {
        
        public:
        using id_t = unsigned int; 
        using Map = std::unordered_map<id_t, ODGameObject>;

        static ODGameObject createGameObject(){
            static id_t currentId = 0;
            return ODGameObject(currentId++);
        };

        static ODGameObject makePointLight(float intensity = 5.0f, float radius =0.1f, glm::vec3 color = glm::vec3(1.f));

        ODGameObject(const ODGameObject&) = delete;
        ODGameObject& operator=(const ODGameObject&) = delete; 
        ODGameObject(ODGameObject&&) = default;
        ODGameObject& operator=(ODGameObject&&) = default;

        const id_t getId() const { return m_id; }

        std::shared_ptr<ODModel> model {}; 
        glm::vec3 color {}; 
        TransformComponent transform{};

        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
            ODGameObject(id_t id) : m_id(id) {}
            id_t m_id;
    };

}