#pragma once

#include "OmniVDevice.hpp"

// std
#include <string>
#include <vector>

namespace OmniV {

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		uint32_t stagesCount = 2;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class OmniVPipeline {
	public:
		OmniVPipeline(OmniVDevice& device, const PipelineConfigInfo& configInfo, const std::string& vertFilepath, const std::string& fragFilepath = "");

		~OmniVPipeline();

		OmniVPipeline(const OmniVPipeline&) = delete;
		OmniVPipeline operator=(const OmniVPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> readFile(const std::string& filename);

		void createGraphicsPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilepath, const std::string& fragFilepath);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		OmniVDevice& omnivDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule = VK_NULL_HANDLE;
		VkShaderModule fragShaderModule = VK_NULL_HANDLE;
	};
}