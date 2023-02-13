#pragma once

#include "Renderer.hpp"
#include "Device.hpp"
#include "Window.hpp"

// std
#include <cassert>

namespace OmniV {
    class ShadowmapRenderer {
    public:
        ShadowmapRenderer(Device& device, Renderer& renderer);
        ~ShadowmapRenderer();

        ShadowmapRenderer(const ShadowmapRenderer&) = delete;
        ShadowmapRenderer& operator=(const ShadowmapRenderer&) = delete;

        VkRenderPass getShadowmapRenderPass() const { return m_renderPass; }

        VkImageView getShadowmapImageView() const { return m_depthImageView; }
        VkSampler getShadowmapSampler() const { return m_shadowmapSampler; }

        void beginShadowmapRenderPass(VkCommandBuffer commandBuffer, uint32_t cascadeIndex);
        void endCurrentRenderPass(VkCommandBuffer commandBuffer);

        VkFormat findDepthFormat();
    
    private:
        void createResources();
        void createRenderPass();

        VkFramebuffer getFrameBuffer(int cascadeIndex) { return m_depthFramebuffers[cascadeIndex]; }
        VkExtent2D getShadowmapExtent() { return VkExtent2D{ SHADOWMAP_RES, SHADOWMAP_RES }; }

        Device& m_device;
        Renderer& m_renderer;

        // Shadowmap pass
        VkRenderPass m_renderPass;

        // Shadowmap pass resources
        VkFramebuffer m_depthFramebuffers[SHADOWMAP_CASCADE_COUNT];

        VkImage m_depthImage;
        VkDeviceMemory m_depthImageMemory;
        VkImageView m_depthImageView;
        VkImageView m_cascadesDepthImageViews[SHADOWMAP_CASCADE_COUNT];

        VkSampler m_shadowmapSampler;
    };
}