#pragma once

#include "ODDevice.h"
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
            ~App();

            App(const App&) = delete;
            App& operator=(const App&) = delete;

            void run();

        private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        private:
            ODWindow m_window{WIDTH, HEIGHT, "Obsidian Engine"};
            ODDevice m_device{m_window};
            ODSwapChain m_swapChain{m_device, m_window.getExtent()};
            std::unique_ptr<ODPipeline> m_pipeline;
            VkPipelineLayout m_pipelineLayout = nullptr;
            std::vector<VkCommandBuffer> m_commandBuffers;

    };
}