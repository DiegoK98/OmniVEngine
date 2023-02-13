#pragma once

#include "Device.hpp"

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

	class Pipeline {
	public:
		Pipeline(Device& device, const PipelineConfigInfo& configInfo, const std::string& vertFilepath, const std::string& fragFilepath = "");

		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline operator=(const Pipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> readFile(const std::string& filename);

		void createGraphicsPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilepath, const std::string& fragFilepath);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		Device& m_device;
		VkPipeline m_graphicsPipeline;
		VkShaderModule m_vertShaderModule = VK_NULL_HANDLE;
		VkShaderModule m_fragShaderModule = VK_NULL_HANDLE;
	};
}