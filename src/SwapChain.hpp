#pragma once

#include "Device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace OmniV {

	class SwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SwapChain(Device& device, VkExtent2D windowExtent);
		SwapChain(Device& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain operator=(const SwapChain&) = delete;

		VkFramebuffer getFrameBuffer(int frameIndex) { return m_framebuffers[frameIndex]; }
		VkRenderPass getRenderPass() { return m_renderPass; }
		VkImageView getImageView(int frameIndex) { return m_imageViews[frameIndex]; }
		size_t imageCount() { return m_images.size(); }
		VkFormat getSwapChainImageFormat() { return m_imageFormat; }
		VkExtent2D getSwapChainExtent() { return m_extent; }

		float extentAspectRatio() {
			return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);
		}
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
		bool compareSwapFormats(const SwapChain& swapChain) const {
			return swapChain.m_depthFormat == m_depthFormat && swapChain.m_imageFormat == m_imageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createDepthResources();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat m_imageFormat;
		VkFormat m_depthFormat;
		VkExtent2D m_extent;

		// Main pass
		std::vector<VkFramebuffer> m_framebuffers;
		VkRenderPass m_renderPass;

		// Main pass resources
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;

		std::vector<VkImage> m_depthImages;
		std::vector<VkDeviceMemory> m_depthImageMemories;
		std::vector<VkImageView> m_depthImageViews;

		Device& m_device;
		VkExtent2D m_viewExtent;

		VkSwapchainKHR m_swapChain;
		std::shared_ptr<SwapChain> m_oldSwapChain;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;
	};

}