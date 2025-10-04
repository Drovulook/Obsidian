#include "app.h"

#include "keyboardMovementController.h"
#include "RendererSystems/SimpleRendererSystem.h"
#include "RendererSystems/PointLightSystem.h"
#include "RendererSystems/GridSystem.h"
#include "ODBuffer.h"

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
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ODSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ODSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    }

    App::~App(){

    }

    void App::run() {

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
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        
        // temporaire: accéder au GameObject de la texture
        ODGameObject* objPtr = nullptr;
        auto it = m_gameObjects.find(static_cast<ODGameObject::id_t>(0));
        if (it != m_gameObjects.end()) {
            objPtr = &it->second; // référence vers l'objet existant
        } else {
            printf("Erreur: pas de GameObject avec id 0\n");
        }

        std::vector<VkDescriptorSet> globalDescriptorSets(ODSwapChain::MAX_FRAMES_IN_FLIGHT); // 1 descriptor set per frame
        for(int i=0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = objPtr->model->getTextureImageView();
            imageInfo.sampler = objPtr->model->getTextureSampler();

            ODDescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRendererSystem simpleRendererSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        PointLightSystem pointLightSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        // GridSystem gridSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

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
            m_cameraObject.camera->updatePerspectiveProjection();

            if(auto commandBuffer = m_renderer.beginFrame()) { // If swapChain needs to be recreated, returns a nullptr
                int frameIndex = m_renderer.getCurrentFrameIndex();
                
                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    *m_cameraObject.camera,
                    globalDescriptorSets[frameIndex],
                    m_gameObjects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = m_cameraObject.camera->getProjection();
                ubo.view = m_cameraObject.camera->getView();
                ubo.inverseView = m_cameraObject.camera->getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                
                // order matters for alpha blending
                simpleRendererSystem.renderGameObjects(frameInfo);
                // gridSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);
            
                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(m_device.device());
    }

    std::shared_ptr<ODModel> App::createModelFromFile(const std::string &modelPath){
        return ODModel::createModelFromFile(m_device, modelPath);
    }
}