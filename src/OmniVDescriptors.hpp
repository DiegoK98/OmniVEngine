#pragma once

#include "OmniVDevice.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace OmniV {

    class OmniVDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(OmniVDevice& omnivDevice) : omnivDevice{ omnivDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<OmniVDescriptorSetLayout> build() const;

        private:
            OmniVDevice& omnivDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        OmniVDescriptorSetLayout(OmniVDevice& omnivDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~OmniVDescriptorSetLayout();
        OmniVDescriptorSetLayout(const OmniVDescriptorSetLayout&) = delete;
        OmniVDescriptorSetLayout& operator=(const OmniVDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        OmniVDevice& omnivDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class OmniVDescriptorWriter;
    };

    class OmniVDescriptorPool {
    public:
        class Builder {
        public:
            Builder(OmniVDevice& omnivDevice) : omnivDevice{ omnivDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<OmniVDescriptorPool> build() const;

        private:
            OmniVDevice& omnivDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        OmniVDescriptorPool(OmniVDevice& omnivDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~OmniVDescriptorPool();
        OmniVDescriptorPool(const OmniVDescriptorPool&) = delete;
        OmniVDescriptorPool& operator=(const OmniVDescriptorPool&) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        OmniVDevice& omnivDevice;
        VkDescriptorPool descriptorPool;

        friend class OmniVDescriptorWriter;
    };

    class OmniVDescriptorWriter {
    public:
        OmniVDescriptorWriter(OmniVDescriptorSetLayout& setLayout, OmniVDescriptorPool& pool);

        OmniVDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        OmniVDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        OmniVDescriptorSetLayout& setLayout;
        OmniVDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}