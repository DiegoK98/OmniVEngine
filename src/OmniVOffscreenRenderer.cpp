#include "OmniVOffscreenRenderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace OmniV {

	OmniVOffscreenRenderer::OmniVOffscreenRenderer(OmniVDevice& device, OmniVRenderer& renderer)
		: omnivDevice{ device }, omnivRenderer{ renderer } {
		createShadowmapResources();
		createShadowmapRenderPass();
	}

	OmniVOffscreenRenderer::~OmniVOffscreenRenderer() {  }

	void OmniVOffscreenRenderer::beginShadowmapRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginShadowmapRenderPass if frame is not in progress");
		assert(commandBuffer == omnivRenderer.getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = getShadowmapRenderPass();
		renderPassInfo.framebuffer = getShadowmapFrameBuffer();

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = getShadowmapExtent();

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(getShadowmapExtent().width);
		viewport.height = static_cast<float>(getShadowmapExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, getShadowmapExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Set depth bias (aka "Polygon offset")
		// Required to avoid shadow mapping artifacts
		vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
	}

	void OmniVOffscreenRenderer::endShadowmapRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

	void OmniVOffscreenRenderer::createShadowmapResources() {
		// Shadowmap depth buffer
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = SHADOWMAP_RES;
		imageInfo.extent.height = SHADOWMAP_RES;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = findDepthFormat();
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		omnivDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowmapDepthImage, shadowmapDepthImageMemory);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = shadowmapDepthImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = findDepthFormat();
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(omnivDevice.device(), &viewInfo, nullptr, &shadowmapDepthImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		// Create sampler to sample from to depth attachment
		// Used to sample in the fragment shader for shadowed rendering
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST; // Choosing linear or nearest depending on if DEPTH_FORMAT supports linear or not
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if (vkCreateSampler(omnivDevice.device(), &samplerInfo, nullptr, &shadowmapSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sampler!");
		}

		// Shadowmap depth FBO
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = shadowmapRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowmapDepthImageView;
		framebufferInfo.width = SHADOWMAP_RES;
		framebufferInfo.height = SHADOWMAP_RES;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(omnivDevice.device(), &framebufferInfo, nullptr, &shadowmapDepthFramebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	void OmniVOffscreenRenderer::createShadowmapRenderPass() {
		// Attachments
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 0;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Subpasses
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// Dependencies
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &depthAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(omnivDevice.device(), &renderPassInfo, nullptr, &shadowmapRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	VkFormat OmniVOffscreenRenderer::findDepthFormat() {
		return omnivDevice.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}