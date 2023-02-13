#pragma once

#include "Camera.hpp"
#include "Device.hpp"
#include "FrameInfo.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace OmniV {
	class RenderSystem {
	public:
		explicit RenderSystem(Device& device) : m_device(device) {}
		~RenderSystem() { vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr); }

		virtual void render(FrameInfo& frameInfo) { std::cerr << "Render function not implemented" << std::endl; };

	protected:
		Device& m_device;

		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	};
}