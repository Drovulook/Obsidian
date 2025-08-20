#pragma once

#include "ODCamera.h"
#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODPipeline.h"

// std
#include <memory>
#include <vector>

namespace ODEngine {
    class SimpleRendererSystem {
        public:

            SimpleRendererSystem(ODDevice& device, VkRenderPass renderPass);
            virtual ~SimpleRendererSystem();

            SimpleRendererSystem(const SimpleRendererSystem&) = delete;
            SimpleRendererSystem& operator=(const SimpleRendererSystem&) = delete;
            
            void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<ODGameObject>& gameObjects, const ODCamera& camera);

        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODPipeline> m_pipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
    };

}