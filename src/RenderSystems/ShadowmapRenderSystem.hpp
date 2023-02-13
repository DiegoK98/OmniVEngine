#pragma once

#include "RenderSystem.hpp"

namespace OmniV {
	class ShadowmapRenderSystem final : public RenderSystem {
	public:
		ShadowmapRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~ShadowmapRenderSystem();

		ShadowmapRenderSystem(const ShadowmapRenderSystem&) = delete;
		ShadowmapRenderSystem& operator=(const ShadowmapRenderSystem&) = delete;

		void render(FrameInfo& frameInfo);

		uint32_t m_activeCascadeIndex = 0;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath = "");
	};
}