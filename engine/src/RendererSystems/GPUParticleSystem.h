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
            
            void render(FrameInfo& frameInfo);

        private:
            void createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createGraphicsPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODGraphicsPipeline> m_pipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
    };

}