# pragma once

#include <vector>
#include <random>

#include <glm/glm.hpp>

namespace ODEngine {
    namespace ODParticles {
        const uint32_t PARTICLE_COUNT = 8192;

        struct Particle {
            glm::vec2 position;
            glm::vec2 velocity;
            glm::vec4 color;
            float size;
        };
    }
}