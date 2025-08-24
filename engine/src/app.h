#pragma once

#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODWindow.h"
#include "ODRenderer.h"

// std
#include <memory>
#include <vector>

namespace ODEngine {
    class App {
        public:
            static constexpr int WIDTH = 1000;
            static constexpr int HEIGHT = 700;

            App();
            explicit App(const std::string& modelPath);
            virtual ~App();

            App(const App&) = delete;
            App& operator=(const App&) = delete;

            virtual void run();

        private:
        void loadGameObjects();

        private:
            ODWindow m_window{WIDTH, HEIGHT, "Obsidian Engine"};
            ODDevice m_device{m_window};
            ODRenderer m_renderer{m_window, m_device};
            std::vector<ODGameObject> m_gameObjects;

            std::string m_modelPath;

    };

    App* CreateApp();
}