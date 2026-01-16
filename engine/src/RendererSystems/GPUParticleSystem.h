#pragma once

#include "Renderer/Common/ODCamera.h"
#include "Renderer/Vulkan/ODDevice.h"
#include "Renderer/Common/ODModel.h"
#include "Renderer/Common/ODGameObject.h"
#include "Renderer/Vulkan/ODPipeline.h"
#include "Renderer/Common/FrameInfo.h"

// std
#include <memory>
#include <vector>

namespace ODEngine {
    class GPUParticleSystem {
        public:

            GPUParticleSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            virtual ~GPUParticleSystem();

            GPUParticleSystem(const GPUParticleSystem&) = delete;
            GPUParticleSystem& operator=(const GPUParticleSystem&) = delete;
            
            void compute(FrameInfo& frameInfo, VkCommandBuffer commandBuffer,
                std::vector<VkSemaphore> computeFinishedSemaphores, std::vector<VkFence> computeInFlightFences);

            void render(FrameInfo& frameInfo);

        private:
            void createComputePipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createComputePipeline(VkRenderPass renderPass);
            void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, FrameInfo& frameInfo);

            void createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createGraphicsPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODComputePipeline> m_odComputePipeline;
            VkPipelineLayout m_computePipelineLayout = nullptr;
            std::unique_ptr<ODGraphicsPipeline> m_odGraphicsPipeline;
            VkPipelineLayout m_graphicsPipelineLayout = nullptr;
    };

}