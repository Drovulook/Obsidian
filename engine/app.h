#pragma once

#include "ODWindow.h"
#include "ODPipeline.h"
#include "ODDevice.h"

namespace ODEngine {
    class App {
        public:
            static constexpr int WIDTH = 1000;
            static constexpr int HEIGHT = 700;

            void run();

        private:
            ODWindow m_window{WIDTH, HEIGHT, "Obsidian Engine"};
            ODDevice m_ODDevice{m_window};
            ODPipeline m_pipeline{
                m_device, 
                "engine/shaders/simple_shader.vert.spv", 
                "engine/shaders/simple_shader.frag.spv",
                 ODPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}