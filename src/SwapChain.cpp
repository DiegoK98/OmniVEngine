#include "SwapChain.hpp"

// std
#include <cstdlib>
#include <cstring>
#include <limits>

namespace OmniV {

	SwapChain::SwapChain(Device& device, VkExtent2D extent)
		: m_device{ device }, m_viewExtent{ extent } {
		init();
	}

	SwapChain::SwapChain(Device& device, VkExtent2D extent, std::shared_ptr<SwapChain> previous)
		: m_device{ device }, m_viewExtent{ extent }, m_oldSwapChain{ previous } {
		init();
		m_oldSwapChain = nullptr;
	}

	void SwapChain::init() {
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDepthResources();
		createFramebuffers();
		createSyncObjects();
	}

	SwapChain::~SwapChain() {
		for (auto imageView : m_imageViews) {
			vkDestroyImageView(m_device.device(), imageView, nullptr);
		}
		m_imageViews.clear();

		if (m_swapChain != nullptr) {
			vkDestroySwapchainKHR(m_device.device(), m_swapChain, nullptr);
			m_swapChain = nullptr;
		}

		for (int i = 0; i < m_depthImages.size(); i++) {
			vkDestroyImageView(m_device.device(), m_depthImageViews[i], nullptr);
			vkDestroyImage(m_device.device(), m_depthImages[i], nullptr);
			vkFreeMemory(m_device.device(), m_depthImageMemories[i], nullptr);
		}

		for (auto framebuffer : m_framebuffers) {
			vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
		}
	}

	VkResult SwapChain::acquireNextImage(uint32_t* imageIndex) {
		vkWaitForFences(
			m_device.device(),
			1,
			&m_inFlightFences[m_currentFrame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(
			m_device.device(),
			m_swapChain,
			std::numeric_limits<uint64_t>::max(),
			m_imageAvailableSemaphores[m_currentFrame],  // must be a not signaled semaphore
			VK_NULL_HANDLE,
			imageIndex);

		return result;
	}

	VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
		if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);
		if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = imageIndex;

		auto result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	void SwapChain::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = m_device.getSwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_device.surface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;      // Optional
			createInfo.pQueueFamilyIndices = nullptr;  // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = m_oldSwapChain == nullptr ? VK_NULL_HANDLE : m_oldSwapChain->m_swapChain;

		if (vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		// we only specified a minimum number of images in the swap chain, so the implementation is
		// allowed to create a swap chain with more. That's why we'll first query the final number of
		// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		// retrieve the handles.
		vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, nullptr);
		m_images.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, m_images.data());

		m_imageFormat = surfaceFormat.format;
		m_extent = extent;
	}

	void SwapChain::createImageViews() {
		m_imageViews.resize(m_images.size());
		for (size_t i = 0; i < m_images.size(); i++) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_imageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageViews[i]) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void SwapChain::createRenderPass() {
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = getSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// You can have several subpasses in the same render pass (deferred rendering/lighting)
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// Subpasses might depend on attachments from other subpasses (or external renderpasses).
		// Note: You can have several dependencies for just 1 subpass (if it depends on external previous passes and external later passes)
		VkSubpassDependency dependency = {};
		
		// Dst pass depends on src pass
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		// What pipeline stages will be waiting (dst) and which ones will be waited for (src)
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		
		// Access scopes (Memory access types used) of each pass, so the GPU can treat data properly
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void SwapChain::createDepthResources() {
		VkFormat depthFormat = findDepthFormat();
		m_depthFormat = depthFormat;
		VkExtent2D swapChainExtent = getSwapChainExtent();

		m_depthImages.resize(imageCount());
		m_depthImageMemories.resize(imageCount());
		m_depthImageViews.resize(imageCount());

		for (int i = 0; i < m_depthImages.size(); i++) {
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = swapChainExtent.width;
			imageInfo.extent.height = swapChainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			m_device.createImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_depthImages[i],
				m_depthImageMemories[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_depthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void SwapChain::createFramebuffers() {
		m_framebuffers.resize(imageCount());

		for (size_t i = 0; i < imageCount(); i++) {
			std::array<VkImageView, 2> attachments = { m_imageViews[i], m_depthImageViews[i] };

			VkExtent2D swapChainExtent = getSwapChainExtent();
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(
				m_device.device(),
				&framebufferInfo,
				nullptr,
				&m_framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void SwapChain::createSyncObjects() {
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	/// <summary>
	/// <para> Choose between the different present modes (V-Sync) </para>
	/// <para> ------------------------------------------------------------------------------------------------------ </para>
	/// <para> - Immediate mode: No synchronization is performed before presenting the frame.
	/// *LOW LATENCY* but visible *TEARING*, as the GPU is always presenting new frames. This also means *HIGH POWER CONSUMPTION* (not good for mobile devices) </para>
	/// <para> - Mailbox mode: When a new refresh cycle hits, the swapchain chooses the latest frame available from the back buffer/s, meaning
	/// *LOW LATENCY*. In order to always have the latest frame, the GPU never idles (waits), meaning *HIGH POWER CONS.* </para>
	/// <para> - FIFO mode (ALWAYS SUPPORTED): When a new refresh cycle hits, the swapchain chooses the next back buffer in order (First In First Out).
	/// If GPU finishes a frame early, it idles until the next refresh cycle, resulting in *LOW POWER CONS.*, but *HIGH LATENCY* as the frame presented may be a few ms old </para>
	/// </summary>
	/// <param name="availablePresentModes"></param>
	/// <returns></returns>
	VkPresentModeKHR SwapChain::chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				OV_DEBUG_LOG("Present mode: Mailbox");
				return availablePresentMode;
			}
		}

		/*for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				LOG("Present mode: Immediate");
				return availablePresentMode;
			}
		}*/

		OV_DEBUG_LOG("Present mode: V-Sync");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = m_viewExtent;
			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	VkFormat SwapChain::findDepthFormat() {
		return m_device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

}