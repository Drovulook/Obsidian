#include "ODRenderer.h"

// std
#include <stdexcept>
#include <array>
#include <chrono>
#include <thread>
#include <iostream>

namespace ODEngine {

    ODRenderer::ODRenderer(ODWindow& window, ODDevice& device)
        : m_window(window), m_device(device) {
        recreateSwapChain();
        createCommandBuffers(); // Déjà ODRendererelé dans recreateSwapChain()
    }

    ODRenderer::~ODRenderer(){
        freeCommandBuffers();
    }

    void ODRenderer::recreateSwapChain() {
        auto extent = m_window.getExtent();
        while(extent.width == 0 || extent.height == 0) { // si une dimension est nulle, attendre que la fenêtre soit redimensionnée
            extent = m_window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device.device());

        if (m_swapChain == nullptr) {
            m_swapChain = std::make_unique<ODSwapChain>(m_device, extent);
        } else {
            std::shared_ptr<ODSwapChain> oldSwapChain = std::move(m_swapChain);
            m_swapChain = std::make_unique<ODSwapChain>(m_device, extent, oldSwapChain);
            if(!oldSwapChain->compareSwapFormats(*m_swapChain)) {
                throw std::runtime_error("Swap chain image or depth format has changed!");
            }
            // if(m_swapChain->imageCount() != m_commandBuffers.size()) {
            //     freeCommandBuffers();
            //     createCommandBuffers();
            // }
        }
    }

    void ODRenderer::createCommandBuffers()
    {
        // Libérer les anciens command buffers s'ils existent
        if (!m_commandBuffers.empty()) {
            vkFreeCommandBuffers(
                m_device.device(), 
                m_device.getCommandPool(), 
                static_cast<uint32_t>(m_commandBuffers.size()), 
                m_commandBuffers.data()
            );
        }
        
        // Allocate command buffers  
        m_commandBuffers.resize(m_swapChain->imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        
        // Ne plus pré-enregistrer - sera fait dans drawFrame()
    }

    void ODRenderer::freeCommandBuffers(){
        vkFreeCommandBuffers(
            m_device.device(), 
            m_device.getCommandPool(), 
            static_cast<uint32_t>(m_commandBuffers.size()), 
            m_commandBuffers.data()
        );
        m_commandBuffers.clear();
    }

    VkCommandBuffer ODRenderer::beginFrame(){
        assert(!m_isFrameStarted && "Cannot call beginFrame while a frame is already in progress!");
        
        auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;  
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // Attendre que cette image soit disponible pour l'enregistrement
        m_swapChain->waitForImageToBeAvailable(m_currentImageIndex);

        m_isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        // vkResetCommandBuffer(commandBuffer, 0); // !! à enlever ?
        
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Command buffer réutilisable

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }

    void ODRenderer::endFrame(){
        assert(m_isFrameStarted && "Cannot call endFrame while no frame is in progress!");
        
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
        }
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }


        m_isFrameStarted = false;
        m_currentFrameIndex = (m_currentFrameIndex + 1) % ODSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void ODRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(m_isFrameStarted && "Cannot call beginSwapChainRenderPass when frame is not in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0075f, 0.0f, 0.0f, 1.0f}; // Clear color; index 0 is the color attachment
        clearValues[1].depthStencil = {1.0f, 0}; // Clear depth; index 1 is the depth attachment
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
         
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void ODRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(m_isFrameStarted && "Cannot call endSwapChainRenderPass when frame is not in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!");
    
        vkCmdEndRenderPass(commandBuffer);
    }

}