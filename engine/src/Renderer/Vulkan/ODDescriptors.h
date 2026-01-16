#pragma once
 
#include "ODDevice.h"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace ODEngine {
 
    class ODDescriptorSetLayout {
        public:
            class Builder {
            public:
                Builder(ODDevice &device) : device{device} {}
            
                Builder &addBinding(
                    uint32_t binding,
                    VkDescriptorType descriptorType,
                    VkShaderStageFlags stageFlags,
                    uint32_t count = 1);
                std::unique_ptr<ODDescriptorSetLayout> build() const;
            
            private:
                ODDevice &device;
                std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
            };
    
            ODDescriptorSetLayout(
                ODDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
            ~ODDescriptorSetLayout();
            ODDescriptorSetLayout(const ODDescriptorSetLayout &) = delete;
            ODDescriptorSetLayout &operator=(const ODDescriptorSetLayout &) = delete;
            
            VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    
        private:
            ODDevice &device;
            VkDescriptorSetLayout descriptorSetLayout;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
            
            friend class ODDescriptorWriter;
    };
    


    class ODDescriptorPool {
        public:
            class Builder {
                public:
                    Builder(ODDevice &device) : device{device} {}
                
                    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
                    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
                    Builder &setMaxSets(uint32_t count);
                    std::unique_ptr<ODDescriptorPool> build() const;
            
                private:
                    ODDevice &device;
                    std::vector<VkDescriptorPoolSize> poolSizes{};
                    uint32_t maxSets = 1000;
                    VkDescriptorPoolCreateFlags poolFlags = 0;
            };
    
            ODDescriptorPool(
                ODDevice &device,
                uint32_t maxSets,
                VkDescriptorPoolCreateFlags poolFlags,
                const std::vector<VkDescriptorPoolSize> &poolSizes);
            ~ODDescriptorPool();
            ODDescriptorPool(const ODDescriptorPool &) = delete;
            ODDescriptorPool &operator=(const ODDescriptorPool &) = delete;
    
            bool allocateDescriptorSet(
                const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
            
            void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
            
            void resetPool();
    
        private:
            ODDevice &device;
            VkDescriptorPool descriptorPool;
            
            friend class ODDescriptorWriter;
    };
    


    class ODDescriptorWriter {
        public:
            ODDescriptorWriter(ODDescriptorSetLayout &setLayout, ODDescriptorPool &pool);
            
            ODDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
            ODDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
            
            bool build(VkDescriptorSet &set);
            void overwrite(VkDescriptorSet &set);
    
        private:
            ODDescriptorSetLayout &setLayout;
            ODDescriptorPool &pool;
            std::vector<VkWriteDescriptorSet> writes;
    };
 
} 