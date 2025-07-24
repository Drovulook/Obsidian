#pragma once

#include "ODDevice.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // -> valeur de profondeur de 0 à 1
#include <glm/glm.hpp>

// std
#include <vector>

namespace ODEngine {
    class ODModel {
        public:

        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        ODModel(ODDevice& device, const std::vector<Vertex>& vertices);
        ~ODModel();
        
        ODModel(const ODModel&) = delete;
        ODModel& operator=(const ODModel&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        private:
        void createVertexBuffer(const std::vector<Vertex>& vertices);

        private:
            ODDevice& m_device;
            VkBuffer m_vertexBuffer;
            VkDeviceMemory m_vertexBufferMemory;
            uint32_t m_vertexCount;

    };
}