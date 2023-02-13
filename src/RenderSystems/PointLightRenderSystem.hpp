#pragma once

#include "RenderSystem.hpp"

namespace OmniV {
	class PointLightRenderSystem final : public RenderSystem {
	public:
		PointLightRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightRenderSystem();

		PointLightRenderSystem(const PointLightRenderSystem&) = delete;
		PointLightRenderSystem& operator=(const PointLightRenderSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath = "");
	};
}