#pragma once

#include "ODDevice.h"

#include <string>
#include <vector>

#include <vulkan/vulkan.h> //pas obligatoire (voir ODWindow.h)

namespace ODEngine {
    struct ODPipelineConfigInfo {};

    class ODPipeline {
        public:
            ODPipeline(
                ODDevice& device, 
                const std::string& vertexShaderPath, 
                const std::string& fragmentShaderPath,
                const ODPipelineConfigInfo& configInfo
            );
            ~ODPipeline();

            ODPipeline(const ODPipeline&) = delete;
            ODPipeline& operator=(const ODPipeline&) = delete;

            static ODPipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string& filename);
            
            void createGraphicsPipeline(
                const std::string& vertexShaderPath, 
                const std::string& fragmentShaderPath,
                const ODPipelineConfigInfo& configInfo
            );

            void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);

            ODDevice& m_device;
            VkPipeline m_graphicsODPipeline;
            VkShaderModule m_vertShaderModule;
            VkShaderModule m_fragShaderModule;
    };
}