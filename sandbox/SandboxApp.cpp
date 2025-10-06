#include "SandboxApp.h"
#include <glm/glm.hpp>

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
        // std::shared_ptr<ODModel> floorModel = createModelFromFile("sandbox/models/floor.obj");
        // create3DObjFromFile(glm::vec3(0.0f, .0f, 0.f), glm::vec3(.0f, .0f, .0f), glm::vec3(20.f, 1.f, 20.f), floorModel);

        m_textureHandler->addTexture("sandbox/textures/viking_room.png");
        
        std::shared_ptr<ODModel> roomModel = createModelFromFile("sandbox/models/room.obj");
        create3DObjFromFile(glm::vec3(0.f, 0.f, 0.f), glm::vec3(glm::half_pi<float>(), glm::half_pi<float>(), .0f), glm::vec3(1.f, 1.f, 1.f), roomModel);

        std::shared_ptr<ODModel> vaseModel = createModelFromFile("sandbox/models/smooth_vase.obj");
        for(int i=0; i<10; i++) {
            for(int k=0; k<10; k++) {
            // create3DObjFromFile(glm::vec3((float)i - 5.f, 0.f, (float)k - 5.f), glm::vec3(.0f, .0f, .0f), glm::vec3(2.f, 2.f, 2.f), vaseModel);
            }
        }

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}  //
        };

        for(int i=0; i<lightColors.size(); i++) {
            auto pointLight = ODGameObject::makePointLight(.5f);
            pointLight.color = lightColors[i];
            
            auto rotateRight = glm::rotate(glm::mat4(1.f), (float)i * glm::two_pi<float>() / (float)lightColors.size(), glm::vec3(0.f, -1.f, 0.f));
            pointLight.transform.translation = glm::vec3(rotateRight * glm::vec4(-1.f, -1.25f, -1.f, 1.f));


            m_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

        void SandboxApp::create3DObjFromFile(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<ODModel> model){
            auto gameObject = ODGameObject::createGameObject();
            gameObject.model = model;
            gameObject.transform.translation = translation;
            gameObject.transform.rotation = rotation;
            gameObject.transform.scale = scale;
            m_gameObjects.emplace(gameObject.getId(), std::move(gameObject));
        }
    
        App* CreateApp() {
            return new SandboxApp();
        }
}