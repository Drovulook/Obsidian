#include "SandboxApp.h"

#include <iostream>

namespace ODEngine {
    
    SandboxApp::SandboxApp()
        : App() {
        loadGameObjects();
    }

    SandboxApp::~SandboxApp() {
    }

    void SandboxApp::run() {
        std::cout << "Running Sandbox Application..." << std::endl;
        App::run();
    }

    void SandboxApp::loadGameObjects(){
        std::shared_ptr<ODModel> odModel = createModelFromFile("sandbox/models/room.obj");
        std::cout << "Model loaded from file." << std::endl;
        auto gameObject = ODGameObject::createGameObject();
        gameObject.model = odModel;
        gameObject.transform.translation = glm::vec3(0.0f, 1.0f, 3.0f);
        gameObject.transform.rotation = glm::vec3(glm::half_pi<float>(), glm::half_pi<float>(), .0f);
        gameObject.transform.scale = glm::vec3(1.f, 1.f, 1.f);
        m_gameObjects.push_back(std::move(gameObject));
    }

    App* CreateApp() {
        return new SandboxApp();
    }
}