#pragma once

#include "OmniVRenderer.hpp"
#include "OmniVDevice.hpp"
#include "OmniVWindow.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace OmniV {
    class OmniVShadowmapRenderer {
    public:
        OmniVShadowmapRenderer(OmniVDevice& device, OmniVRenderer& renderer);
        ~OmniVShadowmapRenderer();

        OmniVShadowmapRenderer(const OmniVShadowmapRenderer&) = delete;
        OmniVShadowmapRenderer& operator=(const OmniVShadowmapRenderer&) = delete;

        VkRenderPass getShadowmapRenderPass() const { return renderPass; }

        VkImageView getShadowmapImageView() const { return depthImageView; }
        VkSampler getShadowmapSampler() const { return shadowmapSampler; }

        void beginShadowmapRenderPass(VkCommandBuffer commandBuffer, uint32_t cascadeIndex);
        void endCurrentRenderPass(VkCommandBuffer commandBuffer);

        VkFormat findDepthFormat();
    
    private:
        void createResources();
        void createRenderPass();

        VkFramebuffer getFrameBuffer(int cascadeIndex) { return depthFramebuffers[cascadeIndex]; }
        VkExtent2D getShadowmapExtent() { return VkExtent2D{ SHADOWMAP_RES, SHADOWMAP_RES }; }

        OmniVDevice& omnivDevice;
        OmniVRenderer& omnivRenderer;

        // Shadowmap pass
        VkRenderPass renderPass;

        // Shadowmap pass resources
        VkFramebuffer depthFramebuffers[SHADOWMAP_CASCADE_COUNT];

        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        VkImageView cascadesDepthImageViews[SHADOWMAP_CASCADE_COUNT];

        VkSampler shadowmapSampler;
    };
}