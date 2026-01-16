#include "ODTextureHandler.h"
#include "../Vulkan/ODBuffer.h"
#include "../Vulkan/ODSwapChain.h"

// libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// std
#include <cassert>
#include <iostream>

namespace ODEngine {
    ODTextureHandler::ODTextureHandler(ODDevice &device)
        : m_device(device) {
    }

    ODTextureHandler::~ODTextureHandler() {
        vkDestroySampler(m_device.device(), m_textureSampler, nullptr);
        vkDestroyImageView(m_device.device(), m_textureImageView, nullptr);
        vkDestroyImage(m_device.device(), m_textureImage, nullptr);
        vkFreeMemory(m_device.device(), m_textureImageMemory, nullptr);
    }

    void ODTextureHandler::addTexture(const std::string &filepath) {
        createTextureImage(filepath);
        createTextureImageView();
        createTextureSampler();
    }

    VkDescriptorImageInfo ODTextureHandler::descriptorInfo()
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textureImageView;
        imageInfo.sampler = m_textureSampler;
        return imageInfo;
    }

    void ODTextureHandler::createTextureImage(const std::string &filepath) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        std::cout << "Texture image size: " << imageSize << " bytes\n";
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        ODBuffer stagingBuffer{
            m_device,
            imageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        void* data;
        stagingBuffer.map();
        stagingBuffer.writeToBuffer(pixels, imageSize);
        stbi_image_free(pixels);
        ODSwapChain::createImage(m_device, texWidth, texHeight, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_textureImage, m_textureImageMemory);
        
        m_device.transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
        m_device.copyBufferToImage(stagingBuffer.getBuffer(), m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

        generateMipmaps(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_mipLevels);

    }

    void ODTextureHandler::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice(), imageFormat, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }
        m_device.generateMipmaps(m_textureImage, texWidth, texHeight, m_mipLevels);
    }

    void ODTextureHandler::createTextureImageView() {
        m_textureImageView = ODSwapChain::createImageView(m_device, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);
    }

    void ODTextureHandler::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR; // magnification = oversampling  (nb fragments > nb texels)
        samplerInfo.minFilter = VK_FILTER_LINEAR; // magnification = undersampling (nb fragments < nb texels)
    
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        // ou, pour le désactiver :
        // samplerInfo.anisotropyEnable = VK_FALSE;
        // samplerInfo.maxAnisotropy = 1.0f;
        VkPhysicalDeviceProperties properties = m_device.properties;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // used if addressMode is clamp to border (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // mipmapping
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
}