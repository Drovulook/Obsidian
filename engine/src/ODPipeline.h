#pragma once

#include "ODDevice.h"

#include <string>
#include <vector>

#include <vulkan/vulkan.h> //pas obligatoire (voir ODWindow.h)

namespace ODEngine {
    struct ODPipelineConfigInfo {
        ODPipelineConfigInfo() = default;

        ODPipelineConfigInfo(const ODPipelineConfigInfo&) = delete;
        ODPipelineConfigInfo& operator=(const ODPipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;

    };

    class ODPipeline {
        public:
            ODPipeline(
                ODDevice& device, 
                const std::string& vertexShaderPath, 
                const std::string& fragmentShaderPath,
                const ODPipelineConfigInfo& configInfo
            );
            ODPipeline() = default;

            ~ODPipeline();

            ODPipeline(const ODPipeline&) = delete;
            ODPipeline& operator=(const ODPipeline&) = delete;

            void bind(VkCommandBuffer commandBuffer);

            static void defaultPipelineConfigInfo(ODPipelineConfigInfo& configInfo);

        private:
            static std::vector<char> readFile(const std::string& filename);
            
            void createGraphicsPipeline(
                const std::string& vertexShaderPath, 
                const std::string& fragmentShaderPath,
                const ODPipelineConfigInfo& configInfo
            );

            void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);

            ODDevice& m_device;
            VkPipeline m_graphicsPipeline;
            VkShaderModule m_vertShaderModule;
            VkShaderModule m_fragShaderModule;
    };
}