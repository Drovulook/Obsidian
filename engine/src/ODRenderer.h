#pragma once

#include "ODDevice.h"
#include "ODModel.h"
#include "ODSwapChain.h"
#include "ODWindow.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace ODEngine {
    class ODRenderer {
        public:

            ODRenderer(ODWindow& window, ODDevice& device);
            virtual ~ODRenderer();

            ODRenderer(const ODRenderer&) = delete;
            ODRenderer& operator=(const ODRenderer&) = delete;

            bool isFrameInProgress() const { return m_isFrameStarted; }
            
            VkCommandBuffer getCurrentCommandBuffer() const {
                assert (m_isFrameStarted && "Cannot get command buffer when frame is not in progress!");
                return m_commandBuffers[m_currentFrameIndex];
            }
            VkRenderPass getSwapChainRenderPass() const {
                return m_swapChain->getRenderPass();
            }

            int getCurrentFrameIndex() const { 
                assert(m_isFrameStarted && "Cannot get current frame index when frame is not in progress!");
                return m_currentFrameIndex; }

            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

        private:
            ODWindow& m_window;
            ODDevice& m_device;
            std::unique_ptr<ODSwapChain> m_swapChain;
            std::vector<VkCommandBuffer> m_commandBuffers;

            uint32_t m_currentImageIndex = 0;
            int m_currentFrameIndex = 0;
            bool m_isFrameStarted = false;

    };
}