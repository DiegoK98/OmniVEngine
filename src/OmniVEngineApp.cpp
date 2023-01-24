#include "OmniVEngineApp.hpp"
#include "OmniVBuffer.hpp"
#include "OmniVKeyboardMovementController.hpp"
#include "RenderSystems/OmniVSimpleRenderSystem.hpp"
#include "RenderSystems/OmniVShadowmapRenderSystem.hpp"
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
		globalPool = OmniVDescriptorPool::Builder(omnivDevice)
			.setMaxSets(OmniVSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, OmniVSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.build();
	}

	OmniVEngineApp::~OmniVEngineApp() {}

	void OmniVEngineApp::run() {

		// Global UBOs and their descriptors
		std::vector<std::unique_ptr<OmniVBuffer>> uboBuffers(OmniVSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<OmniVBuffer>(omnivDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		// This is temporary and should eventually be replaced with a proper class for images/samplers (similar to OmniVBuffer)
		VkDescriptorImageInfo shadowmapImageInfo{};
		shadowmapImageInfo.sampler = omnivRenderer.getShadowmapSampler();
		shadowmapImageInfo.imageView = omnivRenderer.getShadowmapImageView();
		shadowmapImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		auto globalSetLayout = OmniVDescriptorSetLayout::Builder(omnivDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(OmniVSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto globalBufferInfo = uboBuffers[i]->descriptorInfo();
			OmniVDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &globalBufferInfo)
				.writeImage(1, &shadowmapImageInfo)
				.build(globalDescriptorSets[i]);
		}

		// Create Render Systems (pipelines). Should be more generalized, in a loop or something.
		// Warning!!!: The RenderSystem objects are constructed even if the rendersystem is not enabled, which is really bad
		OmniVRenderSystem* renderSystems[MAX_CONCURRENT_RENDER_SYSTEMS] = {};

		// Shadowmap system is separated from the rest because we want to render shadowmaps in a different render pass than everything else
		OmniVShadowmapRenderSystem shadowmapRenderSystem{ omnivDevice, omnivRenderer.getShadowmapRenderPass(), globalSetLayout->getDescriptorSetLayout() };

		OmniVSimpleRenderSystem simpleRenderSystem{ omnivDevice, omnivRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
		if (enabledSystems.simpleRenderSystemEnable) renderSystems[0] = &simpleRenderSystem;

		OmniVPointLightRenderSystem pointLightSystem{ omnivDevice, omnivRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
		if (enabledSystems.pointLightRenderSystemEnable) renderSystems[1] = &pointLightSystem;

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

				// Update UBO
				GlobalUbo ubo{};

				ubo.viewMat = camera.getView();
				ubo.inverseViewMat = camera.getInverseView();
				ubo.projMat = camera.getProjection();
				ubo.ambientLight = renderSettings.ambientLight;

				updateLights(frameInfo, ubo);

				// Matrix from light's point of view (directional lights only)
				glm::mat4 depthProjectionMatrix = glm::ortho(-SHADOWMAP_SIZE, SHADOWMAP_SIZE, -SHADOWMAP_SIZE, SHADOWMAP_SIZE, Z_NEAR, Z_FAR);
				glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(-ubo.lights[0].position), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

				ubo.depthBiasMat = depthProjectionMatrix * depthViewMatrix;

				// Upload UBOs
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Offscreen render pass
				omnivRenderer.beginShadowmapRenderPass(commandBuffer);

				// Render all shadowmaps
				shadowmapRenderSystem.render(frameInfo);

				omnivRenderer.endRenderPass(commandBuffer);

				// Scene render pass
				omnivRenderer.beginSwapChainRenderPass(commandBuffer);

				for (unsigned i = 0; i < enabledSystems.getCount(); i++)
					renderSystems[i]->render(frameInfo);

				omnivRenderer.endRenderPass(commandBuffer);

				omnivRenderer.endFrame();
			}
		}

		// Wait until all operations are completed before closing the window
		vkDeviceWaitIdle(omnivDevice.device());
	}

	void OmniVEngineApp::loadScene(const std::string& sceneFile) {
		std::string scenePath = WORKING_DIR "scenes/" + sceneFile;

		pugi::xml_document doc;

		// Load file to doc
		if (!doc.load_file(scenePath.c_str()))
			throw std::runtime_error("Error while parsing " + sceneFile);

		// Scene node
		auto sceneNode = doc.child("scene");

		///// Checks before parsing /////
		// Scene
		size_t count = std::distance(doc.children("scene").begin(), doc.children("scene").end());

		if (count == 0)
			throw std::runtime_error("No scene defined");
		else if (count > 1)
			throw std::runtime_error("Multiple scenes are not allowed");

		// Render Settings
		count = std::distance(sceneNode.children("rendersettings").begin(), sceneNode.children("rendersettings").end());

		if (count == 0)
			throw std::runtime_error("No render settings defined");
		else if (count > 1)
			throw std::runtime_error("Multiple rendersettings not allowed");

		// Camera
		count = std::distance(sceneNode.children("camera").begin(), sceneNode.children("camera").end());

		if (count == 0)
			throw std::runtime_error("No camera defined");
		else if (count > 1)
			throw std::runtime_error("Multiple cameras are not allowed (bound to change)");

		/* Create scene object */

		// Render Settings parsing
		renderSettings = RenderSettings::loadRenderSettings(sceneNode.child("rendersettings"));

		// Camera parsing
		camera = OmniVCamera::loadCameraFromNode(sceneNode.child("camera"));

		// Meshes parsing
		std::shared_ptr<OmniVModel> omnivModel;
		for (pugi::xml_node meshNode = sceneNode.child("mesh"); meshNode; meshNode = meshNode.next_sibling("mesh"))
		{
			if (!meshNode.attribute("type"))
				throw std::runtime_error("Mesh without type defined");

			if (strcmp(meshNode.attribute("type").value(), "obj") == 0)
			{
				if (!meshNode.find_child_by_attribute("name", "filename"))
					throw std::runtime_error("Obj no defined");

				auto gameObject = OmniVGameObject::createGameObject();

				std::string objPath = meshNode.find_child_by_attribute("name", "filename").attribute("value").value();
				omnivModel = OmniVModel::createModelFromFile(omnivDevice, objPath);
				gameObject.model = omnivModel;
				gameObject.transform.initializeFromNode(meshNode.child("transform"));

				gameObjects.emplace(gameObject.getId(), std::move(gameObject));
			}

			assert(gameObjects.size() < MAX_GAME_OBJECTS && "Exceeded maximum number of objects in scene");
		}

		// Lights parsing
		for (pugi::xml_node lightNode = sceneNode.child("light"); lightNode; lightNode = lightNode.next_sibling("light"))
		{
			bool drawBillboard = lightNode.child("billboard") ? toBool(lightNode.child("billboard").attribute("enabled").value()) : false;

			auto lightGameObject = OmniVGameObject::loadLightFromNode(lightNode, drawBillboard);

			gameObjects.emplace(lightGameObject.getId(), std::move(lightGameObject));

			if (!enabledSystems.pointLightRenderSystemEnable) enabledSystems.pointLightRenderSystemEnable = drawBillboard;

			assert(gameObjects.size() < MAX_GAME_OBJECTS && "Exceeded maximum number of objects in scene");
		}
	}

	// User-defined function
	void OmniVEngineApp::updateLights(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// Continious rotation for lights
		auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });

		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			// Maybe it would be better to just upload up to MAX_LIGHTS lights, ordering them by priority before uploading
			assert(lightIndex < MAX_LIGHTS && "Exceeded maximum number of lights in scene");

			if (obj.directionalLight != nullptr)
			{
				// copy light to ubo
				ubo.lights[lightIndex].type = Directional;
				ubo.lights[lightIndex].position = glm::vec4(obj.directionalLight->direction, 0.f);
				ubo.lights[lightIndex].color = glm::vec4(obj.color, obj.directionalLight->lightIntensity);

				lightIndex += 1;
			}

			if (obj.pointLight != nullptr)
			{
				// update light position
				if (ROTATE_LIGHTS)
					obj.transform.position = glm::vec3(rotateLight * glm::vec4(obj.transform.position, 1.f));

				// copy light to ubo
				ubo.lights[lightIndex].type = Point;
				ubo.lights[lightIndex].position = glm::vec4(obj.transform.position, 1.f);
				ubo.lights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
				ubo.lights[lightIndex].radius = obj.transform.scale.x;

				lightIndex += 1;
			}
		}
		ubo.numLights = lightIndex;
	}
}