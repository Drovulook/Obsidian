#pragma once

#include "ODBuffer.h"
#include "ODDevice.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // -> valeur de profondeur de 0 à 1
#include <glm/glm.hpp>

// std
#include<memory>
#include <vector>

namespace ODEngine {
    class ODModel {
        public:

        struct Vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator== (const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };
        
        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModels(const std::string& filepath);
        };

        ODModel(ODDevice& device, const ODModel::Builder& builder);
        ~ODModel();
        
        ODModel(const ODModel&) = delete;
        ODModel& operator=(const ODModel&) = delete;

        static std::unique_ptr<ODModel> createModelFromFile(ODDevice& device, const std::string& filepath);
        void createTextureImage(ODDevice& device, const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        private:
        void createVertexBuffer(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);

        void createImage(ODDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        private:
            ODDevice& m_device;

            std::unique_ptr<ODBuffer> m_vertexBuffer;
            uint32_t m_vertexCount;

            bool m_hasIndexBuffer = false;
            std::unique_ptr<ODBuffer> m_indexBuffer;
            uint32_t m_indexCount;

            VkImage m_textureImage = VK_NULL_HANDLE;
            VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;

    };
}