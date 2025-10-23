#pragma once

#include "ODCamera.h"
#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODPipeline.h"
#include "FrameInfo.h"

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