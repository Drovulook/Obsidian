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
        std::shared_ptr<ODModel> floorModel = createModelFromFile("sandbox/models/floor.obj");
        create3DObjFromFile(glm::vec3(0.0f, .0f, 0.f), glm::vec3(.0f, .0f, .0f), glm::vec3(20.f, 1.f, 20.f), floorModel);

        // std::shared_ptr<ODModel> odModel = createModelFromFile("sandbox/models/room.obj");
        // auto gameObject = ODGameObject::createGameObject();
        // gameObject.model = odModel;
        // gameObject.transform.translation = glm::vec3(0.0f, 1.0f, 3.0f);
        // gameObject.transform.rotation = glm::vec3(glm::half_pi<float>(), glm::half_pi<float>(), .0f);
        // gameObject.transform.scale = glm::vec3(1.f, 1.f, 1.f);
        // m_gameObjects.push_back(std::move(gameObject));

        std::shared_ptr<ODModel> vaseModel = createModelFromFile("sandbox/models/smooth_vase.obj");
        for(int i=0; i<10; i++) {
            create3DObjFromFile(glm::vec3((float)i, 0.f, 0.f), glm::vec3(.0f, .0f, .0f), glm::vec3(2.f, 2.f, 2.f), vaseModel);
        };
    }

        void SandboxApp::create3DObjFromFile(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<ODModel> model){
            auto gameObject = ODGameObject::createGameObject();
            gameObject.model = model;
            gameObject.transform.translation = translation;
            gameObject.transform.rotation = rotation;
            gameObject.transform.scale = scale;
            m_gameObjects.push_back(std::move(gameObject));
        }
    
        App* CreateApp() {
            return new SandboxApp();
        }
}