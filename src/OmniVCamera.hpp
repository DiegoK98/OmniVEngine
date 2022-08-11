#pragma once

#include "common.hpp"
#include "OmniVGameObject.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "pugixml.hpp"

namespace OmniV {

	class OmniVCamera {
	public:
		OmniVGameObject viewerGameObject = OmniVGameObject::createGameObject();

		void updateMatricesValues(float aspectRatio);

		static OmniVCamera makeCameraFromNode(pugi::xml_node i_camera_node);

		const glm::mat4& getView() const { return viewMatrix; }
		const glm::mat4& getInverseView() const { return inverseViewMatrix; }
		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };

		void setOrthographicProjection();
		void setPerspectiveProjection(float aspect);

		void setViewDirection(glm::vec3 direction);
		void setViewTarget();
		void setViewYXZ();

		float near = 0.1f;
		float far = 100.f;
		bool isPerspective;

		float fovY = glm::radians(50.f);

		float left = 1.f;
		float right = 1.f;
		float bottom = 1.f;
		float top = 1.f;

		// Unfinished behavioiur
		bool useTarget = false;
		Vector3f target;

		Vector3f up = Vector3f(0.f, -1.f, 0.f);
	};
}