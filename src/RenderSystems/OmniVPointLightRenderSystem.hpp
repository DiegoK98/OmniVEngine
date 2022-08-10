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
	class OmniVPointLightRenderSystem {
	public:
		OmniVPointLightRenderSystem(
			OmniVDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~OmniVPointLightRenderSystem();

		OmniVPointLightRenderSystem(const OmniVPointLightRenderSystem&) = delete;
		OmniVPointLightRenderSystem& operator=(const OmniVPointLightRenderSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		OmniVDevice& omnivDevice;

		std::unique_ptr<OmniVPipeline> omnivPipeline;
		VkPipelineLayout pipelineLayout;
	};
}