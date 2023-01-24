#pragma once

#include "OmniVRenderer.hpp"
#include "OmniVDevice.hpp"
#include "OmniVWindow.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace OmniV {
    class OmniVOffscreenRenderer {
    public:
        OmniVOffscreenRenderer(OmniVDevice& device, OmniVRenderer& renderer);
        ~OmniVOffscreenRenderer();

        OmniVOffscreenRenderer(const OmniVOffscreenRenderer&) = delete;
        OmniVOffscreenRenderer& operator=(const OmniVOffscreenRenderer&) = delete;

        VkRenderPass getShadowmapRenderPass() const { return shadowmapRenderPass; }

        VkImageView getShadowmapImageView() const { return shadowmapDepthImageView; }
        VkSampler getShadowmapSampler() const { return shadowmapSampler; }

        void beginShadowmapRenderPass(VkCommandBuffer commandBuffer);
        void endShadowmapRenderPass(VkCommandBuffer commandBuffer);

        VkFormat findDepthFormat();
    
    private:
        void createShadowmapResources();
        void createShadowmapRenderPass();

        VkFramebuffer getShadowmapFrameBuffer() { return shadowmapDepthFramebuffer; }
        VkExtent2D getShadowmapExtent() { return VkExtent2D{ SHADOWMAP_RES, SHADOWMAP_RES }; }

        OmniVDevice& omnivDevice;
        OmniVRenderer& omnivRenderer;

        // Shadowmap pass
        VkRenderPass shadowmapRenderPass;

        // Shadowmap pass resources
        VkFramebuffer shadowmapDepthFramebuffer;

        VkImage shadowmapDepthImage;
        VkDeviceMemory shadowmapDepthImageMemory;
        VkImageView shadowmapDepthImageView;

        VkSampler shadowmapSampler;
    };
}