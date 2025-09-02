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
    class PointLightSystem {
        public:

            PointLightSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            virtual ~PointLightSystem();

            PointLightSystem(const PointLightSystem&) = delete;
            PointLightSystem& operator=(const PointLightSystem&) = delete;
            
            void update(FrameInfo &frameInfo, GlobalUbo &GlobalUbo);
            void render(FrameInfo& frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODPipeline> m_pipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
    };

}