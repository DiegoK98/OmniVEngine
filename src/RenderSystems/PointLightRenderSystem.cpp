#include "PointLightRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

namespace OmniV {

	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightRenderSystem::PointLightRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device) {
		createPipelineLayout(globalSetLayout);

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		Pipeline::enableAlphaBlending(pipelineConfig);
		pipelineConfig.stagesCount = 2;
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		createPipeline(pipelineConfig, "pointLightBillboard.vert.spv", "pointLightBillboard.frag.spv");
	}

	PointLightRenderSystem::~PointLightRenderSystem() {}

	void PointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void PointLightRenderSystem::createPipeline(PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath) {
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pipeline = std::make_unique<Pipeline>(m_device, pipelineConfig, vertFilepath, fragFilepath);
	}

	void PointLightRenderSystem::render(FrameInfo& frameInfo) {
		// sort lights
		std::map<float, GameObject::id_t> sorted;

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.m_pointLight == nullptr || !obj.m_pointLight->drawBillboard) continue;

			// calculate distance
			auto offset = frameInfo.camera.getPosition() - obj.m_transform.position;
			float disSquared = glm::dot(offset, offset);
			sorted[disSquared] = obj.getObjectID();
		}

		m_pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		// iterate through sorted lights in reverse order
		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			// use game obj id to find light object
			auto& obj = frameInfo.gameObjects.at(it->second);

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.m_transform.position, 1.f);
			push.color = glm::vec4(obj.m_color, obj.m_pointLight->lightIntensity);
			push.radius = obj.m_transform.scale.x;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}

}