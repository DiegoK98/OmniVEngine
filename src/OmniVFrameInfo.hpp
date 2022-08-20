#pragma once

#include "OmniVCamera.hpp"
#include "OmniVGameObject.hpp"

// lib
#include <vulkan/vulkan.h>

namespace OmniV {

	struct PointLight {
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
		alignas(16) float radius;
	};

	struct GlobalUbo {
		glm::mat4 viewMat{ 1.f };
		glm::mat4 inverseViewMat{ 1.f };
		glm::mat4 projMat{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		OmniVCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		OmniVGameObject::Map& gameObjects;
	};
}