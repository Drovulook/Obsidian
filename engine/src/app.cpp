#include "app.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // -> valeur de profondeur de 0 à 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <chrono>
#include <thread>
#include <iostream>

namespace ODEngine {
    struct SimplePushConstantData {
        glm::mat2 transform{1.0f};
        glm::vec2 offset;
        alignas(16)glm::vec3 color;
    };

    App::App(){
        loadGameObjects();
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
    void App::loadGameObjects(){
        std::vector<ODModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        auto model = std::make_shared<ODModel>(m_device, vertices);

        std::vector<glm::vec3> colors{
            {1.f, .7f, .73f},
            {1.f, .87f, .73f},
            {1.f, 1.f, .73f},
            {.73f, 1.f, .8f},
            {.73, .88f, 1.f}  
        };
          for (auto& color : colors) {
            color = glm::pow(color, glm::vec3{2.2f});
        }
        // std::vector<ODModel::Vertex> vertices{};
        // SierpinskiTriangle(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

        for (int i = 0; i < 40; i++) {
        auto triangle = ODGameObject::createGameObject();
        triangle.model = model;
        triangle.transform2D.scale = glm::vec2(.5f) + i * 0.025f;
        triangle.transform2D.rotation = i * glm::pi<float>() * .025f;
        triangle.color = colors[i % colors.size()];
        m_gameObjects.push_back(std::move(triangle));
  }
    }
    void App::createPipelineLayout()
    {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; 
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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
            
        renderGameObjects(m_commandBuffers[imageIndex]);

        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void App::renderGameObjects(VkCommandBuffer commandBuffer){
        
        int i = 0;
        for (auto& obj : m_gameObjects) {
            i += 1;
            obj.transform2D.rotation = glm::mod(obj.transform2D.rotation + 0.002f * i, glm::two_pi<float>());
        }

        m_pipeline->bind(commandBuffer);
        for (auto& obj : m_gameObjects) {

            SimplePushConstantData push{};
            push.offset = obj.transform2D.translation;
            push.color = obj.color;
            push.transform = obj.transform2D.mat2d();
            
            vkCmdPushConstants(
                commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, // offset
                sizeof(SimplePushConstantData),
                &push
            );
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
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