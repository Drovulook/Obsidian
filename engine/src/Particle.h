# pragma once

#include "ODBuffer.h"
#include "ODDevice.h"

#include <vector>
#include <random>
#include <array>
#include <memory>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace ODEngine {
    namespace ODParticles {
        const uint32_t PARTICLE_COUNT = 8192 /* * 256 */ ;

        struct Particle {
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
                std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
                bindingDescriptions[0].binding = 0;
                bindingDescriptions[0].stride = sizeof(Particle);
                bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                
                return bindingDescriptions;
            }
            
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
                std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
                attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Particle, position)});
                // !! switcher 1 <-> 0 ?
                attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, color)});
                
                return attributeDescriptions;
            }
            
            glm::vec2 position;
            glm::vec2 velocity;
            glm::vec4 color;
            float size;
        };

        class ParticleSystem {
            public:
                ParticleSystem(ODDevice& device, float width, float height);
                ~ParticleSystem() = default;

                const std::vector<std::unique_ptr<ODBuffer>>& getParticleBuffers() const { return m_particleBuffers; }
            
                private:
                void createParticles();
                void createParticleBuffers();

            private:
                ODDevice& m_device;
                float m_width;
                float m_height;
                
                std::vector<Particle> m_particles;
                std::vector<std::unique_ptr<ODBuffer>> m_particleBuffers;
        };

        
    }
}