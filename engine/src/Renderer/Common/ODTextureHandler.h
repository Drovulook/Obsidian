#pragma once

#include "../Vulkan/ODDevice.h"

// std
#include<memory>

namespace ODEngine {
    class ODTextureHandler {
    public:
        ODTextureHandler(ODDevice& device);
        ~ODTextureHandler();

        ODTextureHandler(const ODTextureHandler&) = delete;
        ODTextureHandler& operator=(const ODTextureHandler&) = delete;

        void addTexture(const std::string& filepath);

        VkDescriptorImageInfo descriptorInfo();
    
    private:
            void createTextureImage(const std::string& filepath);
            void createTextureImageView();
            void createTextureSampler();

            void generateMipmaps(VkImage image,  VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    private:
        ODDevice& m_device;
        
        VkImage m_textureImage = VK_NULL_HANDLE;
        uint32_t m_mipLevels;
        VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
        VkImageView m_textureImageView = VK_NULL_HANDLE;
        VkSampler m_textureSampler = VK_NULL_HANDLE;
    };
}