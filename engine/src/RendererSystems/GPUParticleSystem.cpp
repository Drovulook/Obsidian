#include "GPUParticleSystem.h"
#include "Particle.h"

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
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
        // alignas(16)glm::vec3 color;
    };

    GPUParticleSystem::GPUParticleSystem(ODDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
    : m_device(device) {
        createComputePipelineLayout(globalSetLayout);
        createComputePipeline(renderPass);
    }

    GPUParticleSystem::~GPUParticleSystem(){
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void GPUParticleSystem::createComputePipelineLayout(VkDescriptorSetLayout globalSetLayout){
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); 
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    void GPUParticleSystem::createComputePipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        m_odPipeline.reset();

        ODComputePipelineConfigInfo pipelineConfig{};
        // ODComputePipeline::defaultPipelineConfigInfo(m_device, pipelineConfig);
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_odPipeline = std::make_unique<ODComputePipeline>(
            m_device, 
            ENGINE_PATH "/shaders/compiled/compute.spv", 
            pipelineConfig);
    }

    void GPUParticleSystem::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, FrameInfo& frameInfo) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording compute command buffer!");
        }
        m_odPipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        vkCmdDispatch(commandBuffer, ODParticles::PARTICLE_COUNT / 256, 1, 1);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record compute command buffer!");
        }
    }

    void GPUParticleSystem::compute(FrameInfo& frameInfo, VkCommandBuffer commandBuffer,
         std::vector<VkSemaphore> computeFinishedSemaphores, std::vector<VkFence> computeInFlightFences) {

        vkWaitForFences(m_device.device(), 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        vkResetFences(m_device.device(), 1, &computeInFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        recordComputeCommandBuffer(commandBuffer, frameInfo);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

        if (vkQueueSubmit(m_device.computeQueue(), 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit compute command buffer!");
        };


        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

    }

}