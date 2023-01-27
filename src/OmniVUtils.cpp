#include "OmniVUtils.hpp"

namespace OmniV {

	// Function that adjusts the boundaries of the shadowmap depth pass view matrix to fit the main camera frustum
	// Only used for directional lights
	// from: https://gamedev.stackexchange.com/questions/193929/how-to-move-the-shadow-map-with-the-camera
	glm::mat4 shadowmapAdjustedMatrix(glm::mat4 viewMat, const OmniVCamera& camera, float aspectRatio) {
		float fov = camera.getFovY();
		float nearDist = camera.getNear();
		float farDist = SHADOWMAP_MAX_DIST;
		float Hnear = 2 * tan(fov / 2) * nearDist;
		float Wnear = Hnear * aspectRatio;
		float Hfar = 2 * tan(fov / 2) * farDist;
		float Wfar = Hfar * aspectRatio;

		glm::vec3 camPos = camera.getPosition();
		glm::vec3 camForw = camera.getForward();
		glm::vec3 camUp = camera.getUp();
		glm::vec3 camRight = camera.getRight();

		glm::vec3 centerFar = camPos + camForw * farDist;
		glm::vec3 topLeftFar = centerFar + (camUp * Hfar / 2.0f) - (camRight * Wfar / 2.0f);
		glm::vec3 topRightFar = centerFar + (camUp * Hfar / 2.0f) + (camRight * Wfar / 2.0f);
		glm::vec3 bottomLeftFar = centerFar - (camUp * Hfar / 2.0f) - (camRight * Wfar / 2.0f);
		glm::vec3 bottomRightFar = centerFar - (camUp * Hfar / 2.0f) + (camRight * Wfar / 2.0f);

		glm::vec3 centerNear = camPos + camForw * nearDist;
		glm::vec3 topLeftNear = centerNear + (camUp * Hnear / 2.0f) - (camRight * Wnear / 2.0f);
		glm::vec3 topRightNear = centerNear + (camUp * Hnear / 2.0f) + (camRight * Wnear / 2.0f);
		glm::vec3 bottomLeftNear = centerNear - (camUp * Hnear / 2.0f) - (camRight * Wnear / 2.0f);
		glm::vec3 bottomRightNear = centerNear - (camUp * Hnear / 2.0f) + (camRight * Wnear / 2.0f);

		glm::vec3 frustumCenter = (centerFar - centerNear) * 0.5f;

		std::array<glm::vec3, 8> frustumToLightView
		{
			viewMat * glm::vec4(bottomRightNear, 1.0f),
			viewMat * glm::vec4(topRightNear, 1.0f),
			viewMat * glm::vec4(bottomLeftNear, 1.0f),
			viewMat * glm::vec4(topLeftNear, 1.0f),
			viewMat * glm::vec4(bottomRightFar, 1.0f),
			viewMat * glm::vec4(topRightFar, 1.0f),
			viewMat * glm::vec4(bottomLeftFar, 1.0f),
			viewMat * glm::vec4(topLeftFar, 1.0f)
		};

		// find max and min points to define a ortho matrix around
		glm::vec3 min{ INFINITY, INFINITY, INFINITY };
		glm::vec3 max{ -INFINITY, -INFINITY, -INFINITY };
		for (unsigned int i = 0; i < frustumToLightView.size(); i++)
		{
			if (frustumToLightView[i].x < min.x)
				min.x = frustumToLightView[i].x;
			if (frustumToLightView[i].y < min.y)
				min.y = frustumToLightView[i].y;
			if (frustumToLightView[i].z < min.z)
				min.z = frustumToLightView[i].z;

			if (frustumToLightView[i].x > max.x)
				max.x = frustumToLightView[i].x;
			if (frustumToLightView[i].y > max.y)
				max.y = frustumToLightView[i].y;
			if (frustumToLightView[i].z > max.z)
				max.z = frustumToLightView[i].z;
		}

		float l = min.x;
		float r = max.x;
		float b = min.y;
		float t = max.y;
		// because max.z is positive and in NDC the positive z axis is 
		// towards us so need to set it as the near plane flipped same for min.z.
		float n = -max.z;
		float f = -min.z;

		std::cout << "L:" << l << ", R:" << r << ", B:" << b << ", T:" << t << ", N:" << n << ", F:" << f << std::endl;

		// finally, set our ortho projection
		// and create the light space view-projection matrix
		glm::mat4 projMat = glm::ortho(l, r, b, t, n, f);
		return projMat * viewMat;
	}
}