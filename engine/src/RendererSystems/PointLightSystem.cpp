#include "PointLightSystem.h"

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

    struct PointLightPushConstants {
        glm::vec4 position;
        glm::vec4 color;
        float radius;
    };

    PointLightSystem::PointLightSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
    : m_device(device) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem(){

        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); 
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        m_pipeline.reset();

        ODPipelineConfigInfo pipelineConfig{};
        ODPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<ODPipeline>(
            m_device, 
            ENGINE_PATH "/shaders/point_light_shader.vert.spv", 
            ENGINE_PATH "/shaders/point_light_shader.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo){
        auto rotateRight = glm::rotate(glm::mat4(1.f), 
        frameInfo.frameTime * glm::radians(50.f), 
        glm::vec3(0.f, -1.f, 0.f));

        int lightIndex = 0;
        for(auto& kv: frameInfo.gameObjects) {
            auto& obj = kv.second;
            if(obj.pointLight == nullptr) continue;
            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            obj.transform.translation = glm::vec3(rotateRight * glm::vec4(obj.transform.translation, 1.f));

            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo &frameInfo){

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

        for(auto& kv: frameInfo.gameObjects) {
            auto& obj = kv.second;
            if(obj.pointLight == nullptr) continue;

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.0f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push
            );

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }

    }
}