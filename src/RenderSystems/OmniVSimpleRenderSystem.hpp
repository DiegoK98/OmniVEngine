#pragma once

#include "OmniVRenderSystem.hpp"

namespace OmniV {
	class OmniVSimpleRenderSystem final : public OmniVRenderSystem {
	public:
		OmniVSimpleRenderSystem(OmniVDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~OmniVSimpleRenderSystem();

		OmniVSimpleRenderSystem(const OmniVSimpleRenderSystem&) = delete;
		OmniVSimpleRenderSystem& operator=(const OmniVSimpleRenderSystem&) = delete;

		void render(FrameInfo& frameInfo) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
		void createPipeline(VkRenderPass renderPass) override;
	};
}