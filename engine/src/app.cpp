#include "app.h"

#include "keyboardMovementController.h"
#include "ODCamera.h"
#include "SimpleRendererSystem.h"
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

    struct GlobalUbo {
        glm::mat4 projectionView{1.f};
        glm::vec4 ambientLightColor{1.f, .9f, .9f, .02f};
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f, 1.f, 1.f, 10.f};
    };

    App::App() {
        m_globalDescriptorPool = ODDescriptorPool::Builder(m_device)
            .setMaxSets(ODSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ODSwapChain::MAX_FRAMES_IN_FLIGHT)
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
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
            .build();
        
        std::vector<VkDescriptorSet> globalDescriptorSets(ODSwapChain::MAX_FRAMES_IN_FLIGHT); // 1 descriptor set per frame
        for(int i=0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            ODDescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRendererSystem simpleRendererSystem(m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        ODCamera camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewerObject = ODGameObject::createGameObject();
        viewerObject.transform.translation = {0.f, -1.f, -2.f};
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!m_window.shouldClose()) {
            glfwPollEvents(); 

            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            deltaTime = glm::min(deltaTime, 1.0f / 30.0f);

            cameraController.MoveInPlaneXZ(m_window.getGLFWWindow(), deltaTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = m_renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
            
            if(auto commandBuffer = m_renderer.beginFrame()) { // If swapChain needs to be recreated, returns a nullptr
                int frameIndex = m_renderer.getCurrentFrameIndex();
                
                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex]
                };

                // render
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRendererSystem.renderGameObjects(frameInfo, m_gameObjects);
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