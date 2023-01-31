#pragma once

#include "OmniVDevice.hpp"
#include "OmniVSwapChain.hpp"
#include "OmniVWindow.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace OmniV {
    class OmniVRenderer {
    public:
        OmniVRenderer(OmniVWindow& window, OmniVDevice& device);
        ~OmniVRenderer();

        OmniVRenderer(const OmniVRenderer&) = delete;
        OmniVRenderer& operator=(const OmniVRenderer&) = delete;

        VkRenderPass getRenderPass() const { return omnivSwapChain->getRenderPass(); }

        float getAspectRatio() const { return omnivSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginRenderPass(VkCommandBuffer commandBuffer);
        void endRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        OmniVWindow& omnivWindow;
        OmniVDevice& omnivDevice;
        std::unique_ptr<OmniVSwapChain> omnivSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;
    };
}