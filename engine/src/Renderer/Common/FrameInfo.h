#pragma once

#include "ODCamera.h"
#include "ODGameObject.h"

//lib
#include <vulkan/vulkan.h>

namespace ODEngine {

    # define MAX_LIGHTS 10

    struct PointLight {
        glm::vec4 position{};
        glm::vec4 color{};
    };

    struct GlobalUbo {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{1.f, .9f, .9f, .02f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        ODCamera& camera;
        VkDescriptorSet globalDescriptorSet;
        ODGameObject::Map& gameObjects;
        VkBuffer particleBuffer;
    };
    
    struct ComputeShaderUbo {
        float deltaTime;
    };


    struct ComputeShaderFrameInfo {
        float deltaTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet computeDescriptorSet;
    };
}
