#pragma once

#include "OmniVDevice.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace OmniV {

	class OmniVSwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		OmniVSwapChain(OmniVDevice& deviceRef, VkExtent2D windowExtent);
		OmniVSwapChain(OmniVDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<OmniVSwapChain> previous);
		~OmniVSwapChain();

		OmniVSwapChain(const OmniVSwapChain&) = delete;
		OmniVSwapChain operator=(const OmniVSwapChain&) = delete;

		VkFramebuffer getFrameBuffer(int frameIndex) { return swapChainFramebuffers[frameIndex]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int frameIndex) { return swapChainImageViews[frameIndex]; }
		size_t imageCount() { return swapChainImages.size(); }
		VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }

		float extentAspectRatio() {
			return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
		}
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
		bool compareSwapFormats(const OmniVSwapChain& swapChain) const {
			return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
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

		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		// Main pass
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkRenderPass renderPass;

		// Main pass resources
		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemories;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;

		OmniVDevice& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<OmniVSwapChain> oldSwapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};

}