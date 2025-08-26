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
    class SimpleRendererSystem {
        public:

            SimpleRendererSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            virtual ~SimpleRendererSystem();

            SimpleRendererSystem(const SimpleRendererSystem&) = delete;
            SimpleRendererSystem& operator=(const SimpleRendererSystem&) = delete;
            
            void renderGameObjects(FrameInfo& frameInfo, std::vector<ODGameObject>& gameObjects);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODPipeline> m_pipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
    };

}