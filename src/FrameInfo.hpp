﻿#pragma once

#include "Camera.hpp"
#include "GameObject.hpp"

// libs
#include <vulkan/vulkan.h>

namespace OmniV {

	enum LightType {
		Directional = 0,
		Point = 1,
	};

	struct Light {
		alignas(16) LightType type = Directional;
		alignas(16) glm::vec4 position{}; // ignore w. For directional lights, the position vector is treated as its direction
		alignas(16) glm::vec4 color{}; // w is intensity
		alignas(16) float radius = 0.1f;
	};

	struct GlobalUbo {
		glm::mat4 viewMat{ 1.f };
		glm::mat4 inverseViewMat{ 1.f };
		glm::mat4 projMat{ 1.f };
		glm::mat4 cascadesMats[SHADOWMAP_CASCADE_COUNT];
		float cascadeSplits[SHADOWMAP_CASCADE_COUNT];
		glm::vec4 ambientLight{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		Light lights[MAX_LIGHTS];
		int numLights;

	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
		GameObject::Map& gameObjects;
	};

	struct RenderSettings {
		glm::vec4 ambientLight;

		static RenderSettings loadRenderSettings(pugi::xml_node i_settings_node) {
			RenderSettings renderSettings;

			pugi::xml_node ambientLightNode = i_settings_node.child("ambientlight");
			renderSettings.ambientLight = toVector4f(ambientLightNode.attribute("value").value());

			return renderSettings;
		}
	};
}