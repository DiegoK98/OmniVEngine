#pragma once

#include "GameObject.hpp"

namespace OmniV {

	enum ProjectionType {
		Perspective = 0,
		Orthographic = 1,
	};

	class Camera {
	public:
		GameObject viewerGameObject = GameObject::createGameObject();

		void updateMatricesValues(float aspectRatio);

		static Camera loadCameraFromNode(pugi::xml_node i_camera_node);

		const glm::mat4& getView() const { return m_viewMatrix; }
		const glm::mat4& getInverseView() const { return m_inverseViewMatrix; }
		const glm::mat4& getProjection() const { return m_projectionMatrix; }
		const glm::vec3 getPosition() const { return glm::vec3(m_inverseViewMatrix[3]); }
		const glm::vec3 getForward() const { return glm::vec3(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]); }
		const glm::vec3 getUp() const { return glm::vec3(m_viewMatrix[0][1], m_viewMatrix[1][1], m_viewMatrix[2][1]); }
		const glm::vec3 getRight() const { return glm::vec3(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]); }
		const float getNear() const { return m_near; }
		const float getFar() const { return m_far; }
		const float getFovY() const { return fovY; }

	private:
		glm::mat4 m_projectionMatrix{ 1.f };
		glm::mat4 m_viewMatrix{ 1.f };
		glm::mat4 m_inverseViewMatrix{ 1.f };

		void setOrthographicProjection();
		void setPerspectiveProjection(float aspect);

		void setViewDirection(glm::vec3 direction);
		void setViewTarget();
		void setViewYXZ();

		float m_near = 0.1f;
		float m_far = 100.0f;
		ProjectionType m_projectionType = Perspective;

		float fovY = glm::radians(50.f);

		float left = 1.f;
		float right = 1.f;
		float bottom = 1.f;
		float top = 1.f;

		// Unfinished behavioiur
		bool useTarget = false;
		Vector3f target{ 0.f, 0.f, 0.f };

		Vector3f up{ 0.f, -1.f, 0.f };
	};
}