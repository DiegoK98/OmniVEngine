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
	class OmniVRenderSystem {
	public:
		explicit OmniVRenderSystem(OmniVDevice& device) : omnivDevice(device) {}
		~OmniVRenderSystem() { vkDestroyPipelineLayout(omnivDevice.device(), pipelineLayout, nullptr); }

		OmniVRenderSystem(const OmniVRenderSystem&) = delete;
		OmniVRenderSystem& operator=(const OmniVRenderSystem&) = delete;

		virtual void render(FrameInfo& frameInfo, uint32_t pipelineID = 0) = 0;

	private:
		virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) = 0;
		virtual void createPipeline(uint32_t pipelineID, PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath = "") = 0;

	protected:
		OmniVDevice& omnivDevice;

		std::unique_ptr<OmniVPipeline> omnivPipeline;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	};
}