#include "OmniVEngineApp.hpp"
#include "OmniVBuffer.hpp"
#include "OmniVKeyboardMovementController.hpp"
#include "OmniVFrameInfo.hpp"
#include "OmniVCamera.hpp"
#include "RenderSystems/OmniVSimpleRenderSystem.hpp"
#include "RenderSystems/OmniVPointLightRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

		loadGameObjects();
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

		// Create Game Objects
		auto viewerObject = OmniVGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;

		// Create camera and controller
		OmniVCamera camera{};
		OmniVKeyboardMovementController cameraController{};

		// Main loop
		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!omnivWindow.shouldClose()) {

			glfwPollEvents(); // Process all pending events (window related)

			// Time management
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// View Matrix update (Player movement & rotation)
			cameraController.moveInPlaneXZ(omnivWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			// Projection Matrix update (aspect-ratio)
			float aspect = omnivRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			// Frame
			if (auto commandBuffer = omnivRenderer.beginFrame()) {
				int frameIndex = omnivRenderer.getFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects };

				// Global UBO update & upload
				GlobalUbo ubo{};
				ubo.viewMat = camera.getView();
				ubo.inverseViewMat = camera.getInverseView();
				pointLightSystem.update(frameInfo, ubo);
				ubo.projMat = camera.getProjection();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// RenderPass (Begin Pass -> Draw -> End Pass)
				omnivRenderer.beginSwapChainRenderPass(commandBuffer);

				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				omnivRenderer.endSwapChainRenderPass(commandBuffer);
				omnivRenderer.endFrame();
			}
		}

		// Wait until all operations are completed before closing the window
		vkDeviceWaitIdle(omnivDevice.device());
	}

	void OmniVEngineApp::loadGameObjects() {
		std::shared_ptr<OmniVModel> omnivModel = OmniVModel::createModelFromFile(omnivDevice, "flat_vase.obj");

		auto flatVase = OmniVGameObject::createGameObject();
		flatVase.model = omnivModel;
		flatVase.transform.translation = { -.5f, .5f, 0.f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		omnivModel = OmniVModel::createModelFromFile(omnivDevice, "smooth_vase.obj");

		auto smoothVase = OmniVGameObject::createGameObject();
		smoothVase.model = omnivModel;
		smoothVase.transform.translation = { .5f, .5f, 2.5f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		omnivModel = OmniVModel::createModelFromFile(omnivDevice, "quad.obj");
		auto floor = OmniVGameObject::createGameObject();
		floor.model = omnivModel;
		floor.transform.translation = { 0.f, .5f, 0.f };
		floor.transform.scale = { 3.f, 1.f, 3.f };
		gameObjects.emplace(floor.getId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = OmniVGameObject::makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}
}