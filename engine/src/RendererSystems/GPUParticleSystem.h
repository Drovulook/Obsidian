#pragma once

#include "ODCamera.h"
#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODPipeline.h"
#include "ODFrameInfo.h"

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

        private:
            void createComputePipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createComputePipeline(VkRenderPass renderPass);
            void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, FrameInfo& frameInfo);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODComputePipeline> m_odPipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
    };

}