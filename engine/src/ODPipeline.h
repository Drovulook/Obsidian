#pragma once

#include "ODDevice.h"

#include <string>
#include <vector>

#include <vulkan/vulkan.h> //pas obligatoire (voir ODWindow.h)

namespace ODEngine {
    
    ////////////////////////////////////////////// ODBasePipeline //////////////////////////////////////////////
    class ODBasePipeline {
        public:
        ODBasePipeline(ODDevice& device);
        virtual ~ODBasePipeline() = default;
        
        protected:
        void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);
        static std::vector<char> readFile(const std::string& filename);
        
        ODDevice& m_device;
        VkPipeline m_pipeline;
    };
    
    ////////////////////////////////////////// ODGraphicsPipeline //////////////////////////////////////////
    struct ODGraphicsPipelineConfigInfo {
        ODGraphicsPipelineConfigInfo() = default;

        ODGraphicsPipelineConfigInfo(const ODGraphicsPipelineConfigInfo&) = delete;
        ODGraphicsPipelineConfigInfo& operator=(const ODGraphicsPipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
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

    class ODGraphicsPipeline : public ODBasePipeline {
        public:
            ODGraphicsPipeline(
                ODDevice& device, 
                const std::string& vertexShaderPath, 
                const std::string& fragmentShaderPath,
                const ODGraphicsPipelineConfigInfo& configInfo
            );
            
            ODGraphicsPipeline() = default;
            ~ODGraphicsPipeline() override;

            ODGraphicsPipeline(const ODGraphicsPipeline&) = delete;
            ODGraphicsPipeline& operator=(const ODGraphicsPipeline&) = delete;

            void bind(VkCommandBuffer commandBuffer);

            static void defaultPipelineConfigInfo(ODDevice& device, ODGraphicsPipelineConfigInfo& configInfo);
            static void enableAlphaBlending(ODGraphicsPipelineConfigInfo& configInfo);

        private:
            void createGraphicsPipeline(
                const std::string& vertexShaderPath, 
                const std::string& fragmentShaderPath,
                const ODGraphicsPipelineConfigInfo& configInfo
            );

            VkShaderModule m_vertShaderModule;
            VkShaderModule m_fragShaderModule;
        };
    
    ////////////////////////////////////////// ODComputePipeline ///////////////////////////////////////////
    struct ODComputePipelineConfigInfo {
        ODComputePipelineConfigInfo() = default;

        ODComputePipelineConfigInfo(const ODComputePipelineConfigInfo&) = delete;
        ODComputePipelineConfigInfo& operator=(const ODComputePipelineConfigInfo&) = delete;

        VkPipelineLayout pipelineLayout = nullptr;

    };

    class ODComputePipeline : public ODBasePipeline {
        public:
        ODComputePipeline(
            ODDevice& device, 
            const std::string &computeShaderPath, 
            const ODComputePipelineConfigInfo& configInfo);
            
            ODComputePipeline() = default;
            ~ODComputePipeline() override;

            ODComputePipeline(const ODGraphicsPipeline&) = delete;
            ODComputePipeline& operator=(const ODGraphicsPipeline&) = delete;

            // static void defaultPipelineConfigInfo(ODDevice& device, ODGraphicsPipelineConfigInfo& configInfo);

            void bind(VkCommandBuffer commandBuffer);
            
        private:
            void createComputePipeline(const std::string &computeShaderPath, const ODComputePipelineConfigInfo &configInfo);
            
            VkShaderModule m_computeShaderModule;

    };

}