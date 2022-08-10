﻿#pragma once

#include "OmniVDescriptors.hpp"
#include "OmniVDevice.hpp"
#include "OmniVGameObject.hpp"
#include "OmniVWindow.hpp"
#include "OmniVRenderer.hpp"

// std
#include <memory>
#include <vector>

namespace OmniV {

	class OmniVEngineApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		OmniVEngineApp();
		~OmniVEngineApp();

		OmniVEngineApp(const OmniVEngineApp&) = delete;
		OmniVEngineApp& operator=(const OmniVEngineApp&) = delete;

		void loadScene(const std::string& i_path);
		void run();

	private:
		OmniVWindow omnivWindow{ WIDTH, HEIGHT, "Hello Vulkan!" };
		OmniVDevice omnivDevice{ omnivWindow };
		OmniVRenderer omnivRenderer{omnivWindow, omnivDevice};

		// Note: Order of declarations matters -> We want the DescriptorPool object to be destroyed before the Device object
		// (objects are created in declaration order & destroyed in reverse declaration order)
		std::unique_ptr<OmniVDescriptorPool> globalPool{};
		OmniVGameObject::Map gameObjects; // Should be part of a scene object
	};
}