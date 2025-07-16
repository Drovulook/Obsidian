#include "ODPipeline.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace ODEngine {
    ODPipeline::ODPipeline(
        ODDevice &device, 
        const std::string &vertexShaderPath, 
        const std::string &fragmentShaderPath, 
        const ODPipelineConfigInfo &configInfo): m_device(device) {
            createGraphicsPipeline(vertexShaderPath, fragmentShaderPath, configInfo);
    }

    ODPipeline::~ODPipeline(){
        // vkDestroyShaderModule(m_device.device(), fragShaderModule, nullptr);
        // vkDestroyShaderModule(m_device.device(), vertShaderModule, nullptr);
        // vkDestroyPipeline(m_device.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> ODPipeline::readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if(!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void ODPipeline::createGraphicsPipeline(const std::string &vertexShaderPath, 
        const std::string &fragmentShaderPath, const ODPipelineConfigInfo &configInfo){
        auto vertexCode = readFile(vertexShaderPath);
        auto fragmentCode = readFile(fragmentShaderPath);

        std::cout << "Vertex shader code size: " << vertexCode.size() << '\n';
        std::cout << "Fragment shader code size: " << fragmentCode.size() << '\n';
    }
    void ODPipeline::createShaderModule(const std::vector<char> &code, VkShaderModule &shaderModule){
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if(vkCreateShaderModule(m_device.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
    }

        ODPipelineConfigInfo ODPipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height){
    ODPipelineConfigInfo configInfo{};
            return configInfo;
    }
}