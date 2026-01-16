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
    class SimpleRendererSystem {
        public:

            SimpleRendererSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            virtual ~SimpleRendererSystem();

            SimpleRendererSystem(const SimpleRendererSystem&) = delete;
            SimpleRendererSystem& operator=(const SimpleRendererSystem&) = delete;
            
            void renderGameObjects(FrameInfo& frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODGraphicsPipeline> m_odPipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
    };

}