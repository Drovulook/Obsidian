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
    }

    App::~App(){

    }

    void App::run() {

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

            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            deltaTime = glm::min(deltaTime, 1.0f / 30.0f);

            cameraController.HandleInputs(m_window.getGLFWWindow(), deltaTime, m_cameraObject);
            m_cameraObject.camera->setViewYXZ(m_cameraObject.transform.translation, m_cameraObject.transform.rotation);
            float aspect = m_renderer.getAspectRatio();
            m_cameraObject.camera->updatePerspectiveProjection(aspect);

            auto commandBuffer = m_renderer.getCurrentCommandBuffer();  
            int frameIndex = m_renderer.getCurrentFrameIndex();
            
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
            
            if(auto commandBuffer = m_renderer.beginFrame()) { // If swapChain needs to be recreated, returns a nullptr

                // render
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                
                // order matters for alpha blending
                gpuParticleSystem.render(frameInfo);
                simpleRendererSystem.renderGameObjects(frameInfo);
                // gridSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);
            
                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        vkDeviceWaitIdle(m_device.device());

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
    void App::debugBuffer(ODDevice &device, VkBuffer srcBuffer, size_t size) {
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