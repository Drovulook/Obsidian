#include "app.h"
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

    App::App(){
        loadGameObjects();
    }

    App::~App(){

    }

    void App::run() {
        SimpleRendererSystem simpleRendererSystem(m_device, m_renderer.getSwapChainRenderPass());

        while(!m_window.shouldClose()) {
            glfwPollEvents(); 
            
            if(auto commandBuffer = m_renderer.beginFrame()) { // If swapChain needs to be recreated, returns a nullptr
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRendererSystem.renderGameObjects(commandBuffer, m_gameObjects);
                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
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

}