#pragma once

#include "Renderer/Vulkan/ODDevice.h"
#include "Renderer/Common/ODModel.h"
#include "Renderer/Common/ODGameObject.h"
#include "Renderer/Common/ODWindow.h"
#include "Renderer/Common/ODRenderer.h"
#include "Renderer/Vulkan/ODDescriptors.h"
#include "Renderer/Common/ODTextureHandler.h"
#include "Renderer/Common/ODCamera.h"
#include "UI/UIManager.h"
#include "Renderer/Common/Particle.h"

// std
#include <memory>
#include <vector>

namespace ODEngine {
    class App {
        public:
            static constexpr int WIDTH = 1000;
            static constexpr int HEIGHT = 700;

            App();
            virtual ~App();

            App(const App&) = delete;
            App& operator=(const App&) = delete;

            virtual void run();

            ODDevice& getDevice() { return m_device; }

        protected:
            std::shared_ptr<ODModel> createModelFromFile(const std::string& modelPath);

        private:
            virtual void loadGameObjects() = 0;
            void createTransitionResources();
            void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, 
                          VkImageLayout oldLayout, VkImageLayout newLayout); 
            void debugBuffer(ODDevice& device, VkBuffer srcBuffer, size_t size);

        private:
            ODWindow m_window{WIDTH, HEIGHT, "Obsidian Engine"};
            ODDevice m_device{m_window};
            std::shared_ptr<UIManager> m_uiManager = std::make_shared<UIManager>();
            ODRenderer m_renderer{m_window, m_device, m_uiManager};
            ODParticles::ParticleSystem m_particleSystem{m_device, WIDTH, HEIGHT};
            std::unique_ptr<ODDescriptorPool> m_globalDescriptorPool{};
            std::shared_ptr<ODGameObject> m_cameraObject = nullptr;


            std::vector<VkSemaphore> m_transitionSemaphores;
            std::vector<VkCommandBuffer> m_transitionCommandBuffers;
            std::vector<VkFence> m_transitionFences;

        protected:
            ODGameObject::Map m_gameObjects;
            std::shared_ptr<ODTextureHandler> m_textureHandler = nullptr;

    };

    App* CreateApp();
}