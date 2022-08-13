#include "OmniVEngineApp.hpp"
#include "OmniVBuffer.hpp"
#include "OmniVKeyboardMovementController.hpp"
#include "RenderSystems/OmniVSimpleRenderSystem.hpp"
#include "RenderSystems/OmniVPointLightRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <pugixml.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace OmniV {

	OmniVEngineApp::OmniVEngineApp() {
		globalPool =
			OmniVDescriptorPool::Builder(omnivDevice)
			.setMaxSets(OmniVSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, OmniVSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
	}

	OmniVEngineApp::~OmniVEngineApp() {}

	void OmniVEngineApp::run() {

		// Global UBOs and their descriptors
		std::vector<std::unique_ptr<OmniVBuffer>> uboBuffers(OmniVSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<OmniVBuffer>(
				omnivDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = OmniVDescriptorSetLayout::Builder(omnivDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(OmniVSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			OmniVDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		// Create Render Systems (pipelines)
		OmniVSimpleRenderSystem simpleRenderSystem{ omnivDevice, omnivRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
		OmniVPointLightRenderSystem pointLightSystem{ omnivDevice, omnivRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

		OmniVRenderSystem* renderSystems[2] = { &simpleRenderSystem, &pointLightSystem };

		// Create player controller
		OmniVKeyboardMovementController viewerController{};

		// Main loop
		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!omnivWindow.shouldClose()) {

			glfwPollEvents(); // Process all pending events (window related)

			// Time management
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Player movement & rotation
			viewerController.moveInPlaneXZ(omnivWindow.getGLFWwindow(), frameTime, camera.viewerGameObject.transform);

			// Update camera matrices (View & Projection)
			camera.updateMatricesValues(omnivRenderer.getAspectRatio());

			// Frame
			if (auto commandBuffer = omnivRenderer.beginFrame()) {
				int frameIndex = omnivRenderer.getFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects };

				// Global UBO update & upload
				GlobalUbo ubo{};
				ubo.viewMat = camera.getView();
				ubo.inverseViewMat = camera.getInverseView();
				updatePointLights(frameInfo, ubo);
				ubo.projMat = camera.getProjection();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// RenderPass (Begin Pass -> Draw -> End Pass)
				omnivRenderer.beginSwapChainRenderPass(commandBuffer);

				for (auto& system : renderSystems)
					system->render(frameInfo);

				omnivRenderer.endSwapChainRenderPass(commandBuffer);
				omnivRenderer.endFrame();
			}
		}

		// Wait until all operations are completed before closing the window
		vkDeviceWaitIdle(omnivDevice.device());
	}

	void OmniVEngineApp::loadScene(const std::string& sceneFile) {
		std::string scenePath = "../scenes/" + sceneFile;

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(scenePath.c_str());

		if (!result)
			throw std::runtime_error("Error while parsing " + sceneFile);

		size_t count = std::distance(doc.children("scene").begin(), doc.children("scene").end());

		if (count == 0)
			throw std::runtime_error("No scene node");

		auto scene_node = doc.child("scene");
		/* Create scene object */

		// Camera parsing
		count = std::distance(scene_node.children("camera").begin(), scene_node.children("camera").end());

		if (count == 0)
			throw std::runtime_error("No camera node");

		camera = OmniVCamera::makeCameraFromNode(scene_node.child("camera"));

		uint32_t entity_id = 0;
		std::shared_ptr<OmniVModel> omnivModel;

		// GameObjects parsing
		for (pugi::xml_node node = scene_node.child("mesh"); node; node = node.next_sibling("mesh"))
		{
			if (!node.attribute("type"))
				throw std::runtime_error("Node without type attribute");

			if (strcmp(node.attribute("type").value(), "obj") == 0)
			{
				if (!node.find_child_by_attribute("name", "filename"))
					throw std::runtime_error("Obj no defined");

				auto gameObject = OmniVGameObject::createGameObject();

				std::string objPath = node.find_child_by_attribute("name", "filename").attribute("value").value();
				omnivModel = OmniVModel::createModelFromFile(omnivDevice, objPath);
				gameObject.model = omnivModel;
				gameObject.transform.initializeFromNode(node.child("transform"));

				gameObjects.emplace(gameObject.getId(), std::move(gameObject));
			}

			// assert that gameObjects size is not more than MAX_NUMBER_OF_OBJECTS
		}

		// Lights parsing
		for (pugi::xml_node node = scene_node.child("light"); node; node = node.next_sibling("light"))
		{
			auto lightGameObject = OmniVGameObject::makeLightFromNode(node);
			lightGameObject.transform.initializeFromNode(node.child("transform"));

			gameObjects.emplace(lightGameObject.getId(), std::move(lightGameObject));

			// assert that gameObjects size is not more than MAX_NUMBER_OF_OBJECTS
		}
	}

	void OmniVEngineApp::updatePointLights(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// Continious rotation for lights
		auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });

		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

			// update light position
			obj.transform.position = glm::vec3(rotateLight * glm::vec4(obj.transform.position, 1.f));

			// copy light to ubo
			ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.position, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

			lightIndex += 1;
		}
		ubo.numLights = lightIndex;
	}
}