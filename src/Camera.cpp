#include "Camera.hpp"

// libs
#include "pugixml.hpp"

// std
#include <cassert>
#include <limits>

namespace OmniV {

	void Camera::updateMatricesValues(float aspectRatio) {
		if (useTarget) setViewTarget();
		else setViewYXZ();

		switch (m_projectionType) {
		case Perspective:
			setPerspectiveProjection(aspectRatio);
			break;
		case Orthographic:
			setOrthographicProjection();
			break;
		}
	}

	/// <summary>
	/// Warning: Ortho cameras currently do not reflect aspect ratio changes
	/// </summary>
	void Camera::setOrthographicProjection() {
		m_projectionMatrix = glm::mat4{ 1.0f };
		m_projectionMatrix[0][0] = 2.f / (right - left);
		m_projectionMatrix[1][1] = 2.f / (bottom - top);
		m_projectionMatrix[2][2] = 1.f / (m_far - m_near);
		m_projectionMatrix[3][0] = -(right + left) / (right - left);
		m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix[3][2] = -m_near / (m_far - m_near);
	}

	void Camera::setPerspectiveProjection(float aspect) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFovy = tan(fovY / 2.f);
		m_projectionMatrix = glm::mat4{ 0.0f };
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix[2][2] = m_far / (m_far - m_near);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(m_far * m_near) / (m_far - m_near);
	}

	/// <summary>
	/// Warning: Unfinished behavioiur
	/// </summary>
	void Camera::setViewDirection(glm::vec3 direction) {
		glm::vec3 position = viewerGameObject.m_transform.position;

		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);

		m_inverseViewMatrix = glm::mat4{ 1.f };
		m_inverseViewMatrix[0][0] = u.x;
		m_inverseViewMatrix[0][1] = u.y;
		m_inverseViewMatrix[0][2] = u.z;
		m_inverseViewMatrix[1][0] = v.x;
		m_inverseViewMatrix[1][1] = v.y;
		m_inverseViewMatrix[1][2] = v.z;
		m_inverseViewMatrix[2][0] = w.x;
		m_inverseViewMatrix[2][1] = w.y;
		m_inverseViewMatrix[2][2] = w.z;
		m_inverseViewMatrix[3][0] = position.x;
		m_inverseViewMatrix[3][1] = position.y;
		m_inverseViewMatrix[3][2] = position.z;
	}

	/// <summary>
	/// Warning: Unfinished behavioiur
	/// </summary>
	void Camera::setViewTarget() {
		setViewDirection(target - viewerGameObject.m_transform.position);
	}

	void Camera::setViewYXZ() {
		glm::vec3 position = viewerGameObject.m_transform.position;
		glm::vec3 rotation = viewerGameObject.m_transform.rotation;

		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };

		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);

		m_inverseViewMatrix = glm::mat4{ 1.f };
		m_inverseViewMatrix[0][0] = u.x;
		m_inverseViewMatrix[0][1] = u.y;
		m_inverseViewMatrix[0][2] = u.z;
		m_inverseViewMatrix[1][0] = v.x;
		m_inverseViewMatrix[1][1] = v.y;
		m_inverseViewMatrix[1][2] = v.z;
		m_inverseViewMatrix[2][0] = w.x;
		m_inverseViewMatrix[2][1] = w.y;
		m_inverseViewMatrix[2][2] = w.z;
		m_inverseViewMatrix[3][0] = position.x;
		m_inverseViewMatrix[3][1] = position.y;
		m_inverseViewMatrix[3][2] = position.z;
	}

	Camera Camera::loadCameraFromNode(pugi::xml_node i_camera_node) {
		auto camera = Camera{};

		if (strcmp(i_camera_node.attribute("type").value(), "perspective") == 0)
		{
			camera.m_projectionType = Perspective;
			float degFovY = i_camera_node.find_child_by_attribute("name", "fov") ? toFloat(i_camera_node.find_child_by_attribute("name", "fov").attribute("value").value()) : 50.f;
			camera.fovY = glm::radians(degFovY);
		}
		else if (strcmp(i_camera_node.attribute("type").value(), "orthographic") == 0)
		{
			camera.m_projectionType = Orthographic;

			//ortho
			if (i_camera_node.find_child_by_attribute("name", "left")) camera.left = toFloat(i_camera_node.find_child_by_attribute("name", "left").attribute("value").value());
			if (i_camera_node.find_child_by_attribute("name", "right")) camera.right = toFloat(i_camera_node.find_child_by_attribute("name", "right").attribute("value").value());
			if (i_camera_node.find_child_by_attribute("name", "bottom")) camera.bottom = toFloat(i_camera_node.find_child_by_attribute("name", "bottom").attribute("value").value());
			if (i_camera_node.find_child_by_attribute("name", "top")) camera.top = toFloat(i_camera_node.find_child_by_attribute("name", "top").attribute("value").value());
		}

		//transformation
		pugi::xml_node node = i_camera_node.child("transform");

		size_t count = std::distance(node.children("lookat").begin(), node.children("lookat").end());

		camera.useTarget = count == 1;

		if (count == 0)
			camera.viewerGameObject.m_transform.initializeFromNode(node);

		if (count > 1)
			throw std::runtime_error("More than 1 lookAt defined for the camera");

		if (auto lookatNode = node.child("lookat"))
		{
			uint32 value_counter = 0;

			if (auto targetAttr = lookatNode.attribute("target"))
			{
				camera.target = toVector3f(targetAttr.value());
				value_counter++;
			}

			if (auto originAttr = lookatNode.attribute("origin"))
			{
				camera.viewerGameObject.m_transform.position = toVector3f(originAttr.value());
				value_counter++;
			}

			if (auto upAttr = lookatNode.attribute("up"))
			{
				camera.up = toVector3f(upAttr.value());
			}

			if (value_counter != 2)
				throw std::runtime_error("LookAt not fully defined or malformed");
		}

		//clipping planes
		if (i_camera_node.child("clip"))
		{
			if (auto farNode = i_camera_node.find_child_by_attribute("name", "far"))
				camera.m_far = toFloat(farNode.attribute("value").value());

			if (auto nearNode = i_camera_node.find_child_by_attribute("name", "near"))
				camera.m_far = toFloat(nearNode.attribute("value").value());
		}

		return camera;
	}
}