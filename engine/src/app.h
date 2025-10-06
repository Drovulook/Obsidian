#pragma once

#include "ODDevice.h"
#include "ODModel.h"
#include "ODGameObject.h"
#include "ODWindow.h"
#include "ODRenderer.h"
#include "ODDescriptors.h"
#include "ODTextureHandler.h"

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

        private:
            ODWindow m_window{WIDTH, HEIGHT, "Obsidian Engine"};
            ODDevice m_device{m_window};
            ODRenderer m_renderer{m_window, m_device};
            std::unique_ptr<ODDescriptorPool> m_globalDescriptorPool{};

            std::shared_ptr<ODGameObject> m_cameraObject = nullptr;

        protected:
            ODGameObject::Map m_gameObjects;
            std::shared_ptr<ODTextureHandler> m_textureHandler = nullptr;

    };

    App* CreateApp();
}