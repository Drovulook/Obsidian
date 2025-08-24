#include "SimpleRendererSystem.h"

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
        glm::mat4 transform{1.0f};
        glm::mat4 normalMatrix{1.0f};
        // alignas(16)glm::vec3 color;
    };

    SimpleRendererSystem::SimpleRendererSystem(ODDevice& device, VkRenderPass renderPass) : m_device(device) {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRendererSystem::~SimpleRendererSystem(){

        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void SimpleRendererSystem::createPipelineLayout()
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
    void SimpleRendererSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        m_pipeline.reset();

        ODPipelineConfigInfo pipelineConfig{};
        ODPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<ODPipeline>(
            m_device, 
            ENGINE_PATH "/shaders/simple_shader.vert.spv", 
            ENGINE_PATH "/shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRendererSystem::renderGameObjects(FrameInfo& frameInfo, std::vector<ODGameObject>& gameObjects){
        
        auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        m_pipeline->bind(frameInfo.commandBuffer);
        
        for (auto& obj : gameObjects) {

            SimplePushConstantData push{};
            push.transform = projectionView * obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, // offset
                sizeof(SimplePushConstantData),
                &push
            );
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

}