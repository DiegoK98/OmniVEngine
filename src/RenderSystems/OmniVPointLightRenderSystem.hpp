#pragma once

#include "OmniVRenderSystem.hpp"

namespace OmniV {
	class OmniVPointLightRenderSystem final : public OmniVRenderSystem {
	public:
		OmniVPointLightRenderSystem(OmniVDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~OmniVPointLightRenderSystem();

		OmniVPointLightRenderSystem(const OmniVPointLightRenderSystem&) = delete;
		OmniVPointLightRenderSystem& operator=(const OmniVPointLightRenderSystem&) = delete;

		void render(FrameInfo& frameInfo) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
		void createPipeline(VkRenderPass renderPass) override;
	};
}