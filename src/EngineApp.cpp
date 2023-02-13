#include "Utils.hpp"
#include "EngineApp.hpp"
#include "Buffer.hpp"
#include "KeyboardMovementController.hpp"
#include "RenderSystems/SimpleRenderSystem.hpp"
#include "RenderSystems/ShadowmapRenderSystem.hpp"
#include "RenderSystems/PointLightRenderSystem.hpp"

// libs
#include <pugixml.hpp>

// std
#include <cassert>
#include <chrono>

namespace OmniV {

	EngineApp::EngineApp() {
		// Build descriptor pool
		m_globalPool = DescriptorPool::Builder(m_device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.build();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {

		// Global UBOs and their descriptors
		std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<Buffer>(m_device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		// This is temporary and should eventually be replaced with a proper class for images/samplers (similar to Buffer)
		VkDescriptorImageInfo shadowmapImageInfo{};
		shadowmapImageInfo.sampler = m_shadowmapRenderer.getShadowmapSampler();
		shadowmapImageInfo.imageView = m_shadowmapRenderer.getShadowmapImageView();
		shadowmapImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		// Build descriptor set layout
		auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		// Build actual descriptor sets
		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto globalBufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *m_globalPool)
				.writeBuffer(0, &globalBufferInfo)
				.writeImage(1, &shadowmapImageInfo)
				.build(globalDescriptorSets[i]);
		}

		// Create Render Systems
		std::vector<std::unique_ptr<RenderSystem>> renderSystems;
		renderSystems.reserve(MAX_CONCURRENT_RENDER_SYSTEMS);

		std::unique_ptr<ShadowmapRenderSystem> shadowmapRenderSystem = nullptr;

		// Shadowmaps
		if (m_enabledSystems.shadowmapRenderSystemEnable)
			shadowmapRenderSystem = std::make_unique<ShadowmapRenderSystem>(m_device, m_shadowmapRenderer.getShadowmapRenderPass(), globalSetLayout->getDescriptorSetLayout());

		// Main system
		if (m_enabledSystems.simpleRenderSystemEnable)
			renderSystems.emplace_back(std::make_unique<SimpleRenderSystem>(m_device, m_renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()));

		// Point lights
		if (m_enabledSystems.pointLightRenderSystemEnable)
			renderSystems.emplace_back(std::make_unique<PointLightRenderSystem>(m_device, m_renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()));

		// Create player controller
		KeyboardMovementController viewerController;

		// Main loop
		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!m_window.shouldClose()) {

			glfwPollEvents(); // Process all pending events (window related)

			// Time management
			const auto newTime = std::chrono::high_resolution_clock::now();
			const float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Player movement & rotation
			viewerController.moveInPlaneXZ(m_window.getGLFWwindow(), frameTime, m_camera.viewerGameObject.m_transform);

			// Update camera matrices (View & Projection)
			m_camera.updateMatricesValues(m_renderer.getAspectRatio());

			// Frame
			if (auto commandBuffer = m_renderer.beginFrame()) {
				int frameIndex = m_renderer.getFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, m_camera, globalDescriptorSets[frameIndex], m_gameObjects };

				GlobalUbo ubo;

				// Update UBO
				ubo.viewMat = m_camera.getView();
				ubo.inverseViewMat = m_camera.getInverseView();
				ubo.projMat = m_camera.getProjection();
				ubo.ambientLight = m_renderSettings.ambientLight;

				updateLights(frameInfo, ubo);

				// Matrix from light's point of view (directional lights only)
				glm::mat4 lightViewMat = glm::lookAt(m_camera.getPosition() + glm::vec3(-ubo.lights[0].position), m_camera.getPosition(), glm::vec3(0.0f, -1.0f, 0.0f));

				getCascadeMatrices(lightViewMat, m_camera, m_renderer.getAspectRatio(), ubo.cascadesMats, ubo.cascadeSplits);

				// Upload UBOs
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Shadowmap render passes
				for (uint32_t i = 0; i < SHADOWMAP_CASCADE_COUNT; i++)
				{
					m_shadowmapRenderer.beginShadowmapRenderPass(commandBuffer, i);

					if (shadowmapRenderSystem)
					{
						shadowmapRenderSystem->m_activeCascadeIndex = i;
						shadowmapRenderSystem->render(frameInfo);
					}

					m_shadowmapRenderer.endCurrentRenderPass(commandBuffer);
				}

				// Swapchain render pass
				m_renderer.beginRenderPass(commandBuffer);

				for (unsigned i = 0; i < renderSystems.size(); i++)
					renderSystems[i]->render(frameInfo);

				m_renderer.endRenderPass(commandBuffer);

				m_renderer.endFrame();
			}
		}

		// Wait until all operations are completed before closing the window
		vkDeviceWaitIdle(m_device.device());
	}

	void EngineApp::loadScene(const std::string& sceneFile) {
		std::string scenePath = "scenes/" + sceneFile;

		pugi::xml_document doc;

		// Load file to doc
		if (!doc.load_file(scenePath.c_str()))
			throw std::runtime_error("Error while parsing " + sceneFile);

		// Scene node
		auto sceneNode = doc.child("scene");

		///// Checks before parsing /////
		// Scene
		size_t count = std::distance(doc.children("scene").begin(), doc.children("scene").end()); // Count of scene nodes

		if (count == 0)
			throw std::runtime_error("No scene defined");
		else if (count > 1)
			throw std::runtime_error("Multiple scenes are not allowed");

		// Render Settings
		count = std::distance(sceneNode.children("rendersettings").begin(), sceneNode.children("rendersettings").end()); // Count of rendersettings nodes

		if (count == 0)
			throw std::runtime_error("No render settings defined");
		else if (count > 1)
			throw std::runtime_error("Multiple rendersettings not allowed");

		// Camera
		count = std::distance(sceneNode.children("camera").begin(), sceneNode.children("camera").end()); // Count of camera nodes

		if (count == 0)
			throw std::runtime_error("No camera defined");
		else if (count > 1)
			throw std::runtime_error("Multiple cameras are not allowed (bound to change)");

		/* Create scene object */

		// Render Settings parsing
		m_renderSettings = RenderSettings::loadRenderSettings(sceneNode.child("rendersettings"));

		// Camera parsing
		m_camera = Camera::loadCameraFromNode(sceneNode.child("camera"));

		// Meshes parsing
		std::shared_ptr<Model> model;
		for (pugi::xml_node meshNode = sceneNode.child("mesh"); meshNode; meshNode = meshNode.next_sibling("mesh"))
		{
			if (!meshNode.attribute("type"))
				throw std::runtime_error("Mesh without type defined");

			if (strcmp(meshNode.attribute("type").value(), "obj") == 0)
			{
				if (!meshNode.find_child_by_attribute("name", "filename"))
					throw std::runtime_error("Obj no defined");

				auto gameObject = GameObject::createGameObject();

				std::string objPath = meshNode.find_child_by_attribute("name", "filename").attribute("value").value();
				model = Model::createModelFromFile(m_device, objPath);
				gameObject.m_model = model;
				gameObject.m_transform.initializeFromNode(meshNode.child("transform"));

				m_gameObjects.emplace(gameObject.getObjectID(), std::move(gameObject));
			}

			assert(m_gameObjects.size() < MAX_GAME_OBJECTS && "Exceeded maximum number of objects in scene");
		}

		// Lights parsing
		for (pugi::xml_node lightNode = sceneNode.child("light"); lightNode; lightNode = lightNode.next_sibling("light"))
		{
			bool drawBillboard = lightNode.child("billboard") ? toBool(lightNode.child("billboard").attribute("enabled").value()) : false;

			auto lightGameObject = GameObject::loadLightFromNode(lightNode, drawBillboard);

			m_gameObjects.emplace(lightGameObject.getObjectID(), std::move(lightGameObject));

			if (!m_enabledSystems.pointLightRenderSystemEnable) m_enabledSystems.pointLightRenderSystemEnable = drawBillboard;

			assert(m_gameObjects.size() < MAX_GAME_OBJECTS && "Exceeded maximum number of objects in scene");
		}
	}

	// User-defined function
	void EngineApp::updateLights(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// Continious rotation for lights
		auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });

		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			// Maybe it would be better to just upload up to MAX_LIGHTS lights, ordering them by priority before uploading
			assert(lightIndex < MAX_LIGHTS && "Exceeded maximum number of lights in scene");

			if (obj.m_directionalLight != nullptr)
			{
				// copy light to ubo
				ubo.lights[lightIndex].type = Directional;
				ubo.lights[lightIndex].position = glm::vec4(obj.m_directionalLight->direction, 0.f);
				ubo.lights[lightIndex].color = glm::vec4(obj.m_color, obj.m_directionalLight->lightIntensity);

				lightIndex += 1;
			}

			if (obj.m_pointLight != nullptr)
			{
				// update light position
				if (ROTATE_LIGHTS)
					obj.m_transform.position = glm::vec3(rotateLight * glm::vec4(obj.m_transform.position, 1.f));

				// copy light to ubo
				ubo.lights[lightIndex].type = Point;
				ubo.lights[lightIndex].position = glm::vec4(obj.m_transform.position, 1.f);
				ubo.lights[lightIndex].color = glm::vec4(obj.m_color, obj.m_pointLight->lightIntensity);
				ubo.lights[lightIndex].radius = obj.m_transform.scale.x;

				lightIndex += 1;
			}
		}
		ubo.numLights = lightIndex;
	}
}