#pragma once

#include "OmniVRenderSystem.hpp"

namespace OmniV {
	class OmniVSimpleRenderSystem final : public OmniVRenderSystem {
	public:
		OmniVSimpleRenderSystem(OmniVDevice& device, VkRenderPass offscreenRenderPass, VkRenderPass sceneRenderPass, VkDescriptorSetLayout globalSetLayout);
		~OmniVSimpleRenderSystem();

		OmniVSimpleRenderSystem(const OmniVSimpleRenderSystem&) = delete;
		OmniVSimpleRenderSystem& operator=(const OmniVSimpleRenderSystem&) = delete;

		void render(FrameInfo& frameInfo, uint32_t pipelineID = 0) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
		void createPipeline(uint32_t pipelineID, PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath = "") override;

		std::unique_ptr<OmniVPipeline> omnivOffscreenPipeline;
	};
}