#include "GridSystem.h"

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
    GridSystem::GridSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
    : m_device(device) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);

        float factor = 10.0f;
        auto gridBuilder = ODModel::Builder{};
        gridBuilder.vertices = {
            {{-factor, 0.0f, -factor}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{factor, 0.0f, -factor}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{factor, 0.0f, factor}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-factor, 0.0f, factor}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        };
        gridBuilder.indices = {0, 1, 2, 2, 3, 0};

        m_grid = std::make_unique<ODModel>(m_device, gridBuilder);
    }

    GridSystem::~GridSystem(){
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void GridSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout){
        // VkPushConstantRange pushConstantRange{};
        // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        // pushConstantRange.offset = 0;
        // pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); 
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    void GridSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        m_pipeline.reset();

        ODGraphicsPipelineConfigInfo pipelineConfig{};
        ODGraphicsPipeline::defaultPipelineConfigInfo(m_device, pipelineConfig);
        ODGraphicsPipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<ODGraphicsPipeline>(
            m_device, 
            ENGINE_PATH "/shaders/compiled/grid_shader.vert.spv", 
            ENGINE_PATH "/shaders/compiled/grid_shader.frag.spv",
            pipelineConfig);
    }

    void GridSystem::render(FrameInfo& frameInfo){

        m_pipeline->bind(frameInfo.commandBuffer);
        
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );
        m_grid->bind(frameInfo.commandBuffer);
        m_grid->draw(frameInfo.commandBuffer);
    }

}