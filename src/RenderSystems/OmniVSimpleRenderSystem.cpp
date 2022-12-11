#include "OmniVSimpleRenderSystem.hpp"

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
	};

	OmniVSimpleRenderSystem::OmniVSimpleRenderSystem(OmniVDevice& device, VkRenderPass offscreenRenderPass, VkRenderPass sceneRenderPass, VkDescriptorSetLayout globalSetLayout)
		: OmniVRenderSystem(device) {
		createPipelineLayout(globalSetLayout);

		PipelineConfigInfo pipelineConfig{};
		OmniVPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.stagesCount = 2;
		pipelineConfig.renderPass = sceneRenderPass;
		createPipeline(0, pipelineConfig, "scene.vert.spv", "scene.frag.spv");

		pipelineConfig.stagesCount = 1;
		pipelineConfig.renderPass = offscreenRenderPass;
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
		createPipeline(1, pipelineConfig, "offscreen.vert.spv");
	}

	OmniVSimpleRenderSystem::~OmniVSimpleRenderSystem() {}

	void OmniVSimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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
		if (vkCreatePipelineLayout(omnivDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void OmniVSimpleRenderSystem::createPipeline(uint32_t pipelineID, PipelineConfigInfo& pipelineConfig, const std::string& vertFilepath, const std::string& fragFilepath) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		pipelineConfig.pipelineLayout = pipelineLayout;
		if (pipelineID == 0)
			omnivPipeline = std::make_unique<OmniVPipeline>(omnivDevice, pipelineConfig, vertFilepath, fragFilepath);
		else
			omnivOffscreenPipeline = std::make_unique<OmniVPipeline>(omnivDevice, pipelineConfig, vertFilepath, fragFilepath);
	}

	void OmniVSimpleRenderSystem::render(FrameInfo& frameInfo, uint32_t pipelineID) {
		// Main scene render
		if (pipelineID == 0)
			omnivPipeline->bind(frameInfo.commandBuffer);
		else
			omnivOffscreenPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			if (obj.model == nullptr)
				continue;

			SimplePushConstantData push{};
			push.modelMat = obj.transform.mat4();
			push.normalMat = obj.transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

}