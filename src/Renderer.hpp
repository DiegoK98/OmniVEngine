#pragma once

#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace OmniV {
    class Renderer {
    public:
        Renderer(Window& window, Device& device);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        VkRenderPass getRenderPass() const { return m_swapChain->getRenderPass(); }

        float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return m_isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
            return m_commandBuffers[m_currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
            return m_currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginRenderPass(VkCommandBuffer commandBuffer);
        void endRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        Window& m_window;
        Device& m_device;
        std::unique_ptr<SwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex;
        int m_currentFrameIndex = 0;
        bool m_isFrameStarted = false;
    };
}