#include "Particle.h"
#include "ODSwapChain.h"

namespace ODEngine {
    namespace ODParticles {
        ParticleSystem::ParticleSystem(ODDevice& device, float width, float height) 
        : m_device{device}, m_width{width}, m_height{height} {
            createParticles();
            createParticleBuffers();
        }

        void ParticleSystem::createParticles() {
            std::default_random_engine rndEngine((unsigned)time(nullptr));
            std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

            m_particles.reserve(PARTICLE_COUNT);
            for (size_t i = 0; i < PARTICLE_COUNT; ++i) {
                Particle p;
                float r = 1.0f * sqrt(rndDist(rndEngine));
                float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
                float x = r * cos(theta) * m_height / m_width;
                float y = r * sin(theta);
                p.position = glm::vec2(x, y);
                p.velocity = glm::normalize(glm::vec2(x,y)) * 0.1f;
                p.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
                m_particles.push_back(p);
            }
        }
        void ParticleSystem::createParticleBuffers() {
            ODBuffer computeStagingBuffer = ODBuffer {
                m_device,
                sizeof(ODParticles::Particle),
                ODParticles::PARTICLE_COUNT,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // vérifier
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            };
            computeStagingBuffer.map();
            computeStagingBuffer.writeToBuffer((void*)m_particles.data());
            
            m_particleBuffers.resize(ODSwapChain::MAX_FRAMES_IN_FLIGHT);
            for (size_t i = 0; i < ODSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            m_particleBuffers[i] = std::make_unique<ODBuffer>( // liste de listes (2*2) plutôt ?
                    m_device,
                    sizeof(ODParticles::Particle),
                    ODParticles::PARTICLE_COUNT,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );
            m_device.copyBuffer(
                    computeStagingBuffer.getBuffer(),
                    m_particleBuffers[i]->getBuffer(),
                    sizeof(ODParticles::Particle) * ODParticles::PARTICLE_COUNT
                );
            }
        }
    }
}