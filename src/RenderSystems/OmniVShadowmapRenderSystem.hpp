#pragma once

#include "OmniVRenderSystem.hpp"

namespace OmniV {
	class OmniVShadowmapRenderSystem final : public OmniVRenderSystem {
	public:
		OmniVShadowmapRenderSystem(OmniVDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~OmniVShadowmapRenderSystem();

		OmniVShadowmapRenderSystem(const OmniVShadowmapRenderSystem&) = delete;
		OmniVShadowmapRenderSystem& operator=(const OmniVShadowmapRenderSystem&) = delete;

		void render(FrameInfo& frameInfo) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
		void createPipeline(PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath = "") override;
	};
}