#include "ShadowmapRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace OmniV {

	struct SimplePushConstantData {
		glm::mat4 modelMat{ 1.f };
		glm::mat4 normalMat{ 1.f };
		uint32_t cascadeIndex = 0;
	};

	ShadowmapRenderSystem::ShadowmapRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device) {
		createPipelineLayout(globalSetLayout);

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.stagesCount = 1;
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.colorBlendInfo.attachmentCount = 0;
		pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;
		pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT; // Prevents peter-panning
		pipelineConfig.dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		pipelineConfig.dynamicStateInfo = {};
		pipelineConfig.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipelineConfig.dynamicStateInfo.pDynamicStates = pipelineConfig.dynamicStateEnables.data();
		pipelineConfig.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(pipelineConfig.dynamicStateEnables.size());
		pipelineConfig.dynamicStateInfo.flags = 0;
		createPipeline(pipelineConfig, "offscreen.vert.spv");
	}

	ShadowmapRenderSystem::~ShadowmapRenderSystem() {}

	void ShadowmapRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

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

	void ShadowmapRenderSystem::createPipeline(PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath) {
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pipeline = std::make_unique<Pipeline>(m_device, pipelineConfig, vertFilepath, fragFilepath);
	}

	void ShadowmapRenderSystem::render(FrameInfo& frameInfo) {
		m_pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			if (obj.m_model == nullptr)
				continue;

			SimplePushConstantData push{};
			push.modelMat = obj.m_transform.mat4();
			push.normalMat = obj.m_transform.normalMatrix();
			push.cascadeIndex = m_activeCascadeIndex;

			vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			obj.m_model->bind(frameInfo.commandBuffer);
			obj.m_model->draw(frameInfo.commandBuffer);
		}
	}

}