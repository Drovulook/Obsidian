#pragma once

#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODPipeline.h"
#include "ODSwapChain.h"
#include "ODWindow.h"

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

        private:
        void loadGameObjects();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);
        void renderGameObjects(VkCommandBuffer commandBuffer);

        void SierpinskiTriangle(
            std::vector<ODModel::Vertex>& vertices, 
            int depth,
            glm::vec2 top, 
            glm::vec2 left, 
            glm::vec2 right);

        private:
            ODWindow m_window{WIDTH, HEIGHT, "Obsidian Engine"};
            ODDevice m_device{m_window};
            std::unique_ptr<ODSwapChain> m_swapChain;
            std::unique_ptr<ODPipeline> m_pipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
            std::vector<VkCommandBuffer> m_commandBuffers;
            std::vector<ODGameObject> m_gameObjects;

    };

    App* CreateApp();
}