#include "app.h"

// std
#include <stdexcept>
#include <array>
#include <chrono>
#include <thread>
#include <iostream>

namespace ODEngine {
    App::App(){
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers(); // Déjà appelé dans recreateSwapChain()
    }

    App::~App(){
        // Libérer les command buffers
        if (!m_commandBuffers.empty()) {
            vkFreeCommandBuffers(
                m_device.device(), 
                m_device.getCommandPool(), 
                static_cast<uint32_t>(m_commandBuffers.size()), 
                m_commandBuffers.data()
            );
        }
        
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void App::run() {
        while(!m_window.shouldClose()) {
            glfwPollEvents(); 
            drawFrame();   
        }
        vkDeviceWaitIdle(m_device.device());
    }
    void App::loadModels(){
        std::vector<ODModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        // std::vector<ODModel::Vertex> vertices{};
        // SierpinskiTriangle(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
        m_model = std::make_unique<ODModel>(m_device, vertices);
    }
    void App::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; 
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    void App::createPipeline() {
        assert(m_swapChain != nullptr && "Swap chain must be initialized before creating the pipeline");
        assert(m_pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        m_pipeline.reset();

        ODPipelineConfigInfo pipelineConfig{};
        ODPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_swapChain->getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<ODPipeline>(
            m_device, 
            ENGINE_PATH "/shaders/simple_shader.vert.spv", 
            ENGINE_PATH "/shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void App::recreateSwapChain() {
        auto extent = m_window.getExtent();
        while(extent.width == 0 || extent.height == 0) { // si une dimension est nulle, attendre que la fenêtre soit redimensionnée
            extent = m_window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device.device());

        // Reset shared pointers pour libérer les ressources
        // m_swapChain.reset();
        
        // Recréer le swap chain
        if (m_swapChain == nullptr) {
            m_swapChain = std::make_unique<ODSwapChain>(m_device, extent);
        } else {
            m_swapChain = std::make_unique<ODSwapChain>(m_device, extent, std::move(m_swapChain));
            if(m_swapChain->imageCount() != m_commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline(); // optimisation potentielle: ne rien faire si render pass compatible
    }

    void App::createCommandBuffers(){
        
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

    void App::freeCommandBuffers(){
        vkFreeCommandBuffers(
            m_device.device(), 
            m_device.getCommandPool(), 
            static_cast<uint32_t>(m_commandBuffers.size()), 
            m_commandBuffers.data()
        );
        m_commandBuffers.clear();
    }

    void App::recordCommandBuffer(int imageIndex) {
        // Reset le command buffer avant de le réenregistrer
        vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
        
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Command buffer réutilisable

        if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0075f, 0.0f, 0.0f, 1.0f}; // Clear color; index 0 is the color attachment
        clearValues[1].depthStencil = {1.0f, 0}; // Clear depth; index 1 is the depth attachment
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
         
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
        vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);
            
        m_pipeline->bind(m_commandBuffers[imageIndex]);
        m_model->bind(m_commandBuffers[imageIndex]);
        m_model->draw(m_commandBuffers[imageIndex]);
        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void App::drawFrame(){
        uint32_t imageIndex;
        auto result = m_swapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;  
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        // Attendre que cette image soit disponible pour l'enregistrement
        m_swapChain->waitForImageToBeAvailable(imageIndex);
        
        // Enregistrer le command buffer pour cette frame
        recordCommandBuffer(imageIndex);
        
        result = m_swapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
    

    // Recursive function to generate Sierpinski triangle vertices
    void App::SierpinskiTriangle(std::vector<ODModel::Vertex> &vertices, int depth, glm::vec2 top, glm::vec2 left, glm::vec2 right){
        if (depth <= 0) {
            vertices.push_back({top, glm::vec3(top.x, top.y, top.x)});
            vertices.push_back({right, glm::vec3(right.x, right.y, right.x)});
            vertices.push_back({left, glm::vec3(left.x, left.y, left.x)});
        } else {
            auto leftTop = 0.5f * (left + top);
            auto rightTop = 0.5f * (right + top);
            auto leftRight = 0.5f * (left + right);
            SierpinskiTriangle(vertices, depth - 1, left, leftRight, leftTop);
            SierpinskiTriangle(vertices, depth - 1, leftRight, right, rightTop);
            SierpinskiTriangle(vertices, depth - 1, leftTop, rightTop, top);
        }   
    }
}