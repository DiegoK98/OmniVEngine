#pragma once

#include "Descriptors.hpp"
#include "Device.hpp"
#include "GameObject.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "ShadowmapRenderer.hpp"
#include "Camera.hpp"
#include "FrameInfo.hpp"

namespace OmniV {

	struct EnabledRenderSystems
	{
		bool shadowmapRenderSystemEnable = true; // Enabled if at least one object casts shadows
		bool simpleRenderSystemEnable = true; // Always enabled
		bool pointLightRenderSystemEnable = false; // Enabled if at least one point light has drawbillboard=True
	};

	class EngineApp {
	public:
		// Should be defined by the rendersettings
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		EngineApp();
		~EngineApp();

		EngineApp(const EngineApp&) = delete;
		EngineApp& operator=(const EngineApp&) = delete;

		void loadScene(const std::string& i_path);
		void run();

	private:
		Window m_window{ WIDTH, HEIGHT, "Hello Vulkan!" };
		Device m_device{ m_window };
		Renderer m_renderer{ m_window, m_device };
		ShadowmapRenderer m_shadowmapRenderer{ m_device, m_renderer };

		// Note: Order of declarations matters -> We want the DescriptorPool object to be destroyed before the Device object
		// (objects are created in declaration order & destroyed in reverse declaration order)
		std::unique_ptr<DescriptorPool> m_globalPool;
		GameObject::Map m_gameObjects; // Should be part of a scene object

		Camera m_camera;
		RenderSettings m_renderSettings;

		EnabledRenderSystems m_enabledSystems;

		void updateLights(FrameInfo& frameInfo, GlobalUbo& ubo);
	};
}