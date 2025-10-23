#pragma once

#include "ODCamera.h"
#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODPipeline.h"
#include "FrameInfo.h"
#include "ODModel.h"

// std
#include <memory>
#include <vector>

namespace ODEngine {
    class GridSystem {
        public:

            GridSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            virtual ~GridSystem();

            GridSystem(const GridSystem&) = delete;
            GridSystem& operator=(const GridSystem&) = delete;
            
            void render(FrameInfo& frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

        private:
            ODDevice& m_device;
            std::unique_ptr<ODGraphicsPipeline> m_odPipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;

            std::unique_ptr<ODModel> m_grid;
    };

}