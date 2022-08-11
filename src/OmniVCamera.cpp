#include "OmniVCamera.hpp"

// libs
#include "pugixml.hpp"

// std
#include <cassert>
#include <limits>

namespace OmniV {

	void OmniVCamera::updateMatricesValues(float aspectRatio) {
		setViewYXZ();
		if (isPerspective) setPerspectiveProjection(aspectRatio);
		else setOrthographicProjection();
	}

	/// <summary>
	/// Warning: Ortho cameras currently do not reflect aspect ratio changes
	/// </summary>
	void OmniVCamera::setOrthographicProjection() {
		projectionMatrix = glm::mat4{ 1.0f };
		projectionMatrix[0][0] = 2.f / (right - left);
		projectionMatrix[1][1] = 2.f / (bottom - top);
		projectionMatrix[2][2] = 1.f / (far - near);
		projectionMatrix[3][0] = -(right + left) / (right - left);
		projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix[3][2] = -near / (far - near);
	}

	void OmniVCamera::setPerspectiveProjection(float aspect) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFovy = tan(fovY / 2.f);
		projectionMatrix = glm::mat4{ 0.0f };
		projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		projectionMatrix[2][2] = far / (far - near);
		projectionMatrix[2][3] = 1.f;
		projectionMatrix[3][2] = -(far * near) / (far - near);
	}

	/// <summary>
	/// Warning: Unfinished behavioiur
	/// </summary>
	void OmniVCamera::setViewDirection(glm::vec3 direction) {
		glm::vec3 position = viewerGameObject.transform.position;

		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);

		inverseViewMatrix = glm::mat4{ 1.f };
		inverseViewMatrix[0][0] = u.x;
		inverseViewMatrix[0][1] = u.y;
		inverseViewMatrix[0][2] = u.z;
		inverseViewMatrix[1][0] = v.x;
		inverseViewMatrix[1][1] = v.y;
		inverseViewMatrix[1][2] = v.z;
		inverseViewMatrix[2][0] = w.x;
		inverseViewMatrix[2][1] = w.y;
		inverseViewMatrix[2][2] = w.z;
		inverseViewMatrix[3][0] = position.x;
		inverseViewMatrix[3][1] = position.y;
		inverseViewMatrix[3][2] = position.z;
	}

	/// <summary>
	/// Warning: Unfinished behavioiur
	/// </summary>
	void OmniVCamera::setViewTarget() {
		setViewDirection(target - viewerGameObject.transform.position);
	}

	void OmniVCamera::setViewYXZ() {
		glm::vec3 position = viewerGameObject.transform.position;
		glm::vec3 rotation = viewerGameObject.transform.rotation;

		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };

		viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);

		inverseViewMatrix = glm::mat4{ 1.f };
		inverseViewMatrix[0][0] = u.x;
		inverseViewMatrix[0][1] = u.y;
		inverseViewMatrix[0][2] = u.z;
		inverseViewMatrix[1][0] = v.x;
		inverseViewMatrix[1][1] = v.y;
		inverseViewMatrix[1][2] = v.z;
		inverseViewMatrix[2][0] = w.x;
		inverseViewMatrix[2][1] = w.y;
		inverseViewMatrix[2][2] = w.z;
		inverseViewMatrix[3][0] = position.x;
		inverseViewMatrix[3][1] = position.y;
		inverseViewMatrix[3][2] = position.z;
	}

	OmniVCamera OmniVCamera::makeCameraFromNode(pugi::xml_node i_camera_node) {
		auto camera = OmniVCamera{};

		if (strcmp(i_camera_node.attribute("type").value(), "perspective") == 0)
		{
			camera.isPerspective = true;
			float degFovY = i_camera_node.find_child_by_attribute("name", "fov") ? toFloat(i_camera_node.find_child_by_attribute("name", "fov").attribute("value").value()) : 50.f;
			camera.fovY = glm::radians(degFovY);
		}
		else if (strcmp(i_camera_node.attribute("type").value(), "orthographic") == 0)
		{
			camera.isPerspective = false;

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
			camera.viewerGameObject.transform.initializeFromNode(node);

		if (count > 1)
			throw std::runtime_error("More than 1 lookAt defined for the camera");

		if (node.child("lookat"))
		{
			throw std::runtime_error("Unfinished behavioiur");

			uint32 value_counter = 0;

			if (node.child("lookat").attribute("target"))
			{
				camera.target = toVector3f(node.child("lookat").attribute("target").value());
				value_counter++;
			}

			if (node.child("lookat").attribute("origin"))
			{
				camera.viewerGameObject.transform.position = toVector3f(node.child("lookat").attribute("origin").value());
				value_counter++;
			}

			if (node.child("lookat").attribute("up"))
			{
				camera.up = toVector3f(node.child("lookat").attribute("up").value());
				value_counter++;
			}

			if (value_counter != 3)
				throw std::runtime_error("LookAt not fully defined or malformed");
		}

		//clipping planes
		if (i_camera_node.child("clip"))
		{
			if (i_camera_node.find_child_by_attribute("name", "far"))
			{
				camera.far = toFloat(i_camera_node.find_child_by_attribute("name", "far").attribute("value").value());
			}

			if (i_camera_node.find_child_by_attribute("name", "near"))
			{
				camera.far = toFloat(i_camera_node.find_child_by_attribute("name", "near").attribute("value").value());
			}
		}

		return camera;
	}
}