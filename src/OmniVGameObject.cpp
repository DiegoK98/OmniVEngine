#include "OmniVGameObject.hpp"

// std
#include <stdexcept>

namespace OmniV {

	glm::mat4 TransformComponent::mat4() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{position.x, position.y, position.z, 1.0f} };
	}

	glm::mat3 TransformComponent::normalMatrix() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.0f / scale;

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			},
		};
	}

	void TransformComponent::initializeFromNode(pugi::xml_node transformNode) {
		for (pugi::xml_node_iterator it = transformNode.begin(); it != transformNode.end(); ++it)
		{
			if (strcmp(it->name(), "scale") == 0)
			{
				if (it->attribute("value"))
					scale = toVector3f(it->attribute("value").value());
				else
					throw std::runtime_error("Transform scale malformed");
			}

			if (strcmp(it->name(), "rotation") == 0)
			{
				// Check for quaternions
				if (it->attribute("value"))
					rotation = toVector3f(it->attribute("eulerAngles").value());
				else
					throw std::runtime_error("Transform rotation malformed");
			}

			if (strcmp(it->name(), "position") == 0)
			{
				if (it->attribute("value"))
					position = toVector3f(it->attribute("value").value());
				else
					throw std::runtime_error("Transform position malformed");
			}
		}
	}

	OmniVGameObject OmniVGameObject::makeSimpleDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity) {
		OmniVGameObject gameObj = OmniVGameObject::createGameObject();

		gameObj.color = color;
		gameObj.directionalLight = std::make_unique<DirectionalLightComponent>();
		gameObj.directionalLight->lightIntensity = intensity;
		gameObj.directionalLight->direction = direction;

		return gameObj;
	}

	OmniVGameObject OmniVGameObject::makeSimplePointLight(bool drawBillboard, glm::vec3 color, float intensity, float radius) {
		OmniVGameObject gameObj = OmniVGameObject::createGameObject();

		gameObj.color = color;
		gameObj.pointLight = std::make_unique<PointLightComponent>();
		gameObj.pointLight->lightIntensity = intensity;
		gameObj.pointLight->drawBillboard = drawBillboard;
		gameObj.transform.scale.x = radius;

		return gameObj;
	}

	OmniVGameObject OmniVGameObject::loadLightFromNode(pugi::xml_node lightNode, bool drawBillboard) {
		// Directional light
		if (strcmp(lightNode.attribute("type").value(), "directional") == 0)
		{
			// Radiance
			if (!lightNode.child("radiance"))
				throw std::runtime_error("Radiance undefinned");

			glm::vec3 m_radiance = toVector3f(lightNode.child("radiance").attribute("value").value());

			// Intensity
			float light_intensity = lightNode.child("intensity") ? toFloat(lightNode.child("intensity").attribute("value").value()) : 5.0f;

			float light_radius = lightNode.child("radius") ? toFloat(lightNode.child("radius").attribute("value").value()) : 0.1f;

			return makeSimpleDirectionalLight(toVector3f(lightNode.child("direction").attribute("value").value()), m_radiance, light_intensity);
		}

		// Point light
		if (strcmp(lightNode.attribute("type").value(), "point") == 0)
		{
			// Radiance
			if (!lightNode.child("radiance"))
				throw std::runtime_error("Radiance undefinned");

			glm::vec3 m_radiance = toVector3f(lightNode.child("radiance").attribute("value").value());

			// Intensity
			float light_intensity = lightNode.child("intensity") ? toFloat(lightNode.child("intensity").attribute("value").value()) : 5.0f;

			float light_radius = lightNode.child("radius") ? toFloat(lightNode.child("radius").attribute("value").value()) : 0.1f;

			OmniVGameObject gameObj = makeSimplePointLight(drawBillboard, m_radiance, light_intensity, light_radius);
			gameObj.transform.initializeFromNode(lightNode.child("transform"));

			return gameObj;
		}

		throw std::runtime_error("Light type not defined");
	}
}