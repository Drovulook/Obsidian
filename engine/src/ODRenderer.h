#pragma once

#include "ODDevice.h"
#include "ODModel.h"
#include "ODSwapChain.h"
#include "ODWindow.h"
#include "UIManager.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace ODEngine {

    class ODRenderer {
        public:

            ODRenderer(ODWindow& window, ODDevice& device, std::shared_ptr<UIManager>);
            virtual ~ODRenderer();

            ODRenderer(const ODRenderer&) = delete;
            ODRenderer& operator=(const ODRenderer&) = delete;

            bool isFrameInProgress() const { return m_isFrameStarted; }
            
            VkCommandBuffer getCurrentCommandBuffer() const {
                return m_commandBuffers[m_currentFrameIndex];
            }

            VkRenderPass getSwapChainRenderPass() const {
                return m_swapChain->getRenderPass();
            }

            ODSwapChain &getSwapChain() { return *m_swapChain; }

            float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }

            int getCurrentFrameIndex() const { 
                return m_currentFrameIndex; }
            VkSemaphore endFrameWithoutPresent(); // Retourne le sémaphore de rendu fini
            void presentFrame(VkSemaphore waitSemaphore); // Présente en attendant le sémaphore
            uint32_t getCurrentImageIndex() const { return m_currentImageIndex; }
            VkSemaphore getRenderFinishedSemaphore() const { 
                return m_swapChain->getRenderFinishedSemaphore(m_currentImageIndex); 
            }

            VkCommandBuffer beginFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

            std::vector<VkSemaphore> getComputeFinishedSemaphores() { return m_swapChain->computeFinishedSemaphores(); }
            std::vector<VkFence> getComputeInFlightFences() { return m_swapChain->computeInFlightFences(); }
            VkCommandBuffer getCurrentComputeCommandBuffers() { 
                return m_computeCommandBuffers[m_currentFrameIndex]; }

            void consumeSemaphore();

        private:
            void createCommandBuffers();
            void createComputeCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

        private:
            ODWindow& m_window;
            ODDevice& m_device;
            std::unique_ptr<ODSwapChain> m_swapChain;
            std::vector<VkCommandBuffer> m_commandBuffers;
            std::vector<VkCommandBuffer> m_computeCommandBuffers;

            uint32_t m_currentImageIndex;
            int m_currentFrameIndex{0};
            bool m_isFrameStarted = false;

            std::shared_ptr<UIManager> m_uiManager;

    };
}