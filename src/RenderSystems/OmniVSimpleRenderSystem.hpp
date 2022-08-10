#pragma once

#include "OmniVCamera.hpp"
#include "OmniVDevice.hpp"
#include "OmniVFrameInfo.hpp"
#include "OmniVGameObject.hpp"
#include "OmniVPipeline.hpp"

// std
#include <memory>
#include <vector>

namespace OmniV {
	class OmniVSimpleRenderSystem {
	public:
		OmniVSimpleRenderSystem(OmniVDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~OmniVSimpleRenderSystem();

		OmniVSimpleRenderSystem(const OmniVSimpleRenderSystem&) = delete;
		OmniVSimpleRenderSystem& operator=(const OmniVSimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		OmniVDevice& omnivDevice;

		std::unique_ptr<OmniVPipeline> omnivPipeline;
		VkPipelineLayout pipelineLayout;
	};
}