#pragma once

#include "ODCamera.h"

//lib
#include <vulkan/vulkan.h>

namespace ODEngine {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        ODCamera& camera;
        VkDescriptorSet globalDescriptorSet;
    };
}
