#include "app.h"

#include "keyboardMovementController.h"
#include "RendererSystems/SimpleRendererSystem.h"
#include "RendererSystems/PointLightSystem.h"
#include "RendererSystems/GridSystem.h"
#include "RendererSystems/GPUParticleSystem.h"
#include "ODBuffer.h"
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

    App::App() {
        m_globalDescriptorPool = ODDescriptorPool::Builder(m_device)
            .setMaxSets(ODSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ODSwapChain::MAX_FRAMES_IN_FLIGHT) // contient les infos de la caméra et de l'éclairage
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ODSwapChain::MAX_FRAMES_IN_FLIGHT) // texture
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ODSwapChain::MAX_FRAMES_IN_FLIGHT * 2) // buffer compute.comp <-> compute.vert et .frag
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ODSwapChain::MAX_FRAMES_IN_FLIGHT) // info de pas de temps pour le compute shader
            .build();
    
        m_textureHandler = std::make_shared<ODTextureHandler>(m_device);

        createTransitionResources();
    }

    App::~App(){
        vkDeviceWaitIdle(m_device.device());

        for (auto semaphore : m_transitionSemaphores) {
            vkDestroySemaphore(m_device.device(), semaphore, nullptr);
        }

        for (auto fence : m_transitionFences) {
            vkDestroyFence(m_device.device(), fence, nullptr);
        }
    
        if (!m_transitionCommandBuffers.empty()) {
            vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(), 
                                static_cast<uint32_t>(m_transitionCommandBuffers.size()), 
                                m_transitionCommandBuffers.data());
        }
    }

    void App::run() {

        m_uiManager->init(
            m_window.getGLFWWindow(),
            m_device.device(),
            m_device.physicalDevice(),
            m_device.findPhysicalQueueFamilies().graphicsAndComputeFamily,
            m_renderer.getSwapChain().getImagesView().data(),
            static_cast<uint32_t>(m_renderer.getSwapChain().getImagesView().size()),
            m_renderer.getSwapChain().getSwapChainImageFormat(),
            m_device.graphicsQueue()
        );

       std::vector<std::unique_ptr<ODBuffer>> uboComputeBuffers(ODSwapChain::MAX_FRAMES_IN_FLIGHT);
       for(int i=0; i < uboComputeBuffers.size(); i++) {
           uboComputeBuffers[i] = std::make_unique<ODBuffer>(
               m_device,
               sizeof(ComputeShaderUbo),
               1,
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
           );
           uboComputeBuffers[i]->map();
       }

        std::vector<std::unique_ptr<ODBuffer>> uboBuffers(ODSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<ODBuffer>(
                m_device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }


        auto globalSetLayout = ODDescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            
            // computer shader binding
            .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
            .addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)

            .addBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
            
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(ODSwapChain::MAX_FRAMES_IN_FLIGHT); // 1 descriptor set per frame
        for(int i=0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto imageInfo = m_textureHandler->descriptorInfo();
            auto computeBufferInfo0 = m_particleSystem.getParticleBuffers()[i]->descriptorInfo();
            auto computeBufferInfo1 = m_particleSystem.getParticleBuffers()[(i + 1) % 2]->descriptorInfo();
            auto coomputeBufferTime = uboComputeBuffers[i]->descriptorInfo();

            ODDescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .writeBuffer(2, &computeBufferInfo0)
                .writeBuffer(3, &computeBufferInfo1)
                .writeBuffer(4, &coomputeBufferTime)
                .build(globalDescriptorSets[i]);
        }

        SimpleRendererSystem simpleRendererSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        PointLightSystem pointLightSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        // GridSystem gridSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        GPUParticleSystem gpuParticleSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

        auto m_cameraObject = ODGameObject::makeCameraObject();
        m_cameraObject.camera->setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        m_cameraObject.transform.translation = {0.f, -1.f, -2.f};
        KeyboardMovementController cameraController{};
        cameraController.init_callbacks(m_window.getGLFWWindow());

        float aspect = m_renderer.getAspectRatio();
        m_cameraObject.camera->setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 1000.0f);

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!m_window.shouldClose()) {
            glfwPollEvents(); 

            auto currentExtent = m_window.getExtent();
            if (currentExtent.width == 0 || currentExtent.height == 0) {
                continue;  // Ignore le rendu si minimisé
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            deltaTime = glm::min(deltaTime, 1.0f / 30.0f);

            cameraController.HandleInputs(m_window.getGLFWWindow(), deltaTime, m_cameraObject);
            m_cameraObject.camera->setViewYXZ(m_cameraObject.transform.translation, m_cameraObject.transform.rotation);
            float aspect = m_renderer.getAspectRatio();
            m_cameraObject.camera->updatePerspectiveProjection(aspect);

            int frameIndex = m_renderer.getCurrentFrameIndex();
            auto commandBuffer = m_renderer.beginFrame();
            if(commandBuffer != nullptr) { // If swapChain needs to be recreated, returns a nullptr
                
                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    *m_cameraObject.camera,
                    globalDescriptorSets[frameIndex],
                    m_gameObjects,
                    m_particleSystem.getParticleBuffers()[(frameIndex + 1) % 2]->getBuffer()
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = m_cameraObject.camera->getProjection();
                ubo.view = m_cameraObject.camera->getView();
                ubo.inverseView = m_cameraObject.camera->getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // compute
                ComputeShaderFrameInfo computeFrameInfo{
                    deltaTime,
                    commandBuffer,
                    globalDescriptorSets[frameIndex]
                };
                
                uboComputeBuffers[frameIndex]->writeToBuffer(&deltaTime);
                uboComputeBuffers[frameIndex]->flush();
                
                // compute
                gpuParticleSystem.compute(frameInfo, m_renderer.getCurrentComputeCommandBuffers(),
                m_renderer.getComputeFinishedSemaphores(),
                m_renderer.getComputeInFlightFences()
                );
                
                // render
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                
                // order matters for alpha blending
                gpuParticleSystem.render(frameInfo);
                simpleRendererSystem.renderGameObjects(frameInfo);
                // gridSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);

                m_renderer.endSwapChainRenderPass(commandBuffer);
                
                VkSemaphore renderFinishedSemaphore = m_renderer.endFrameWithoutPresent();

                if (renderFinishedSemaphore != VK_NULL_HANDLE) {
                    uint32_t currentImageIndex = m_renderer.getCurrentImageIndex();
                    
                    m_uiManager->newFrame();
                    m_uiManager->render();

                    VkSemaphore uiSemaphore = m_uiManager->renderUI(
                        m_device.graphicsQueue(), 
                        currentImageIndex,
                        renderFinishedSemaphore
                    );

                        vkWaitForFences(m_device.device(), 1, &m_transitionFences[currentImageIndex], VK_TRUE, UINT64_MAX);
                        vkResetFences(m_device.device(), 1, &m_transitionFences[currentImageIndex]);
                        
                        VkCommandBuffer transitionCmd = m_transitionCommandBuffers[currentImageIndex];
                        vkResetCommandBuffer(transitionCmd, 0);
        
                        VkCommandBufferBeginInfo beginInfo{};
                        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                        vkBeginCommandBuffer(transitionCmd, &beginInfo);
                        
                        // Transition de layout pour la présentation
                        VkImage swapChainImage = m_renderer.getSwapChain().getImages()[currentImageIndex];
                        transitionImageLayout(transitionCmd, swapChainImage, 
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
                                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
                        
                        vkEndCommandBuffer(transitionCmd);
                        
                        // Soumettre la transition
                        VkSubmitInfo submitInfo{};
                        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                        submitInfo.waitSemaphoreCount = 1;
                        submitInfo.pWaitSemaphores = &uiSemaphore;
                        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        submitInfo.pWaitDstStageMask = &waitStage;
                        submitInfo.commandBufferCount = 1;
                        submitInfo.pCommandBuffers = &transitionCmd;
                        
                        // Créer un nouveau sémaphore pour signaler la fin de la transition
                        VkSemaphore transitionSemaphore = m_transitionSemaphores[currentImageIndex];
                        submitInfo.signalSemaphoreCount = 1;
                        submitInfo.pSignalSemaphores = &transitionSemaphore;
                        
                        vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_transitionFences[currentImageIndex]);

                        // Présenter en attendant que l'UI soit terminée
                        m_renderer.presentFrame(transitionSemaphore);
                } else {
                    // Fallback si l'UI ne retourne pas de sémaphore
                    std::cout << "Warning: UI Manager did not return a semaphore, presenting directly after rendering." << std::endl;
                    m_renderer.presentFrame(renderFinishedSemaphore);
                }
            } else {
                std::cout << "Swap chain recreation in progress, skipping frame rendering." << std::endl;
                m_renderer.consumeSemaphore();
            }

        // size_t bufferSize = sizeof(ODParticles::Particle) * ODParticles::PARTICLE_COUNT;
        // std::cout << "Contents of m_computeBuffers[0]:" << std::endl;
        // debugBuffer(m_device, m_computeBuffers[0]->getBuffer(), bufferSize);
        // std::cout << "Contents of m_computeBuffers[1]:" << std::endl;
        // debugBuffer(m_device, m_computeBuffers[1]->getBuffer(), bufferSize);
        }
        vkDeviceWaitIdle(m_device.device());
    }

    std::shared_ptr<ODModel> App::createModelFromFile(const std::string &modelPath){
        return ODModel::createModelFromFile(m_device, modelPath);
    }

    void App::createTransitionResources(){
        size_t imageCount = m_renderer.getSwapChain().imageCount();

        m_transitionSemaphores.resize(imageCount);
        m_transitionCommandBuffers.resize(imageCount);
        m_transitionFences.resize(imageCount);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Commencer signalé
    
        for (size_t i = 0; i < imageCount; i++) {
            if (vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_transitionSemaphores[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create transition semaphore!");
            }
            if (vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_transitionFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create transition fence!");
            }
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_transitionCommandBuffers.size());
        
        if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_transitionCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate transition command buffers!");
        }
    }

    void App::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage, destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && 
            newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = 0; // Présentation n'a pas besoin d'accès spécial
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else {
            throw std::runtime_error("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void App::debugBuffer(ODDevice &device, VkBuffer srcBuffer, size_t size)
    {
        ODBuffer stagingBuffer{
        device,
        sizeof(ODParticles::Particle),  // Taille par élément
        ODParticles::PARTICLE_COUNT,    // Nombre d'éléments
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    stagingBuffer.map();

    // Copier du buffer device vers staging
    device.copyBuffer(srcBuffer, stagingBuffer.getBuffer(), size);

    // Lire et afficher
    ODParticles::Particle* data = reinterpret_cast<ODParticles::Particle*>(stagingBuffer.getMappedMemory());
    for (size_t i = 0; i < std::min(size_t(10), size_t(ODParticles::PARTICLE_COUNT)); ++i) { // Affiche les 10 premières
        std::cout << "Particle " << i << ": Pos(" << data[i].position.x << ", " << data[i].position.y 
                  << "), Vel(" << data[i].velocity.x << ", " << data[i].velocity.y 
                  << "), Color(" << data[i].color.r << ", " << data[i].color.g << ", " << data[i].color.b << ")" << std::endl;
    }
    stagingBuffer.unmap();  // Optionnel, mais bon
    }
}