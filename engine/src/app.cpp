#include "app.h"

#include "keyboardMovementController.h"
#include "ODCamera.h"
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

    App::App(const std::string& modelPath) : m_modelPath(modelPath) {
        loadGameObjects();
    }

    App::~App(){

    }

    void App::run() {
        SimpleRendererSystem simpleRendererSystem(m_device, m_renderer.getSwapChainRenderPass());
        ODCamera camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewerObject = ODGameObject::createGameObject();
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
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);
            
            if(auto commandBuffer = m_renderer.beginFrame()) { // If swapChain needs to be recreated, returns a nullptr
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRendererSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);
                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(m_device.device());
    }

    void App::loadGameObjects(){
        std::shared_ptr<ODModel> odModel = ODModel::createModelFromFile(m_device, m_modelPath);
        auto gameObject = ODGameObject::createGameObject();
        gameObject.model = odModel;
        gameObject.transform.translation = glm::vec3(0.0f, 0.0f, 2.5f);
        gameObject.transform.scale = glm::vec3(2.f, 2.f, 2.f);
        m_gameObjects.push_back(std::move(gameObject));
    }

}