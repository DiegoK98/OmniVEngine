#pragma once

#include "RenderSystem.hpp"

namespace OmniV {
	class SimpleRenderSystem final : public RenderSystem {
	public:
		SimpleRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath = "");

		std::unique_ptr<Pipeline> m_offscreenPipeline;
	};
}