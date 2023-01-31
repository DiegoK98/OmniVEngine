#include "OmniVUtils.hpp"

namespace OmniV {

	// Function that adjusts the boundaries of the shadowmap depth pass view matrix to fit the main camera frustum
	// Only used for directional lights
	// from: https://gamedev.stackexchange.com/questions/193929/how-to-move-the-shadow-map-with-the-camera
	CascadesGroup shadowmapCascadesMatrices(glm::mat4 viewMat, const OmniVCamera& camera, float aspectRatio) {
		
		CascadesGroup cascades = {};

		float cascadeSplits[SHADOWMAP_CASCADE_COUNT];

		glm::vec3 camPos = camera.getPosition();
		glm::vec3 camForw = camera.getForward();
		glm::vec3 camUp = camera.getUp();
		glm::vec3 camRight = camera.getRight();

		float fov = camera.getFovY();
		float nearClip = camera.getNear();
		float farClip = SHADOWMAP_MAX_DIST;
		float clipRange = farClip - nearClip;
		float frustumRatio = farClip / nearClip;

		// Splits should be editable like in Unity, instead of this distribution
		/*for (uint32_t i = 0; i < SHADOWMAP_CASCADE_COUNT; i++) {
			float splitPercent = (i + 1) / static_cast<float>(SHADOWMAP_CASCADE_COUNT);
			float log = nearClip * std::pow(frustumRatio, splitPercent);
			float uniform = nearClip + clipRange * splitPercent;
			float d = SHADOWMAP_CASCADE_LAMBDA * (log - uniform) + uniform;
			cascadeSplits[i] = d - nearClip;
		}*/

		cascadeSplits[0] = 6.25;
		cascadeSplits[1] = 12.5;
		cascadeSplits[2] = 25;
		cascadeSplits[3] = 50;

		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOWMAP_CASCADE_COUNT; i++)
		{
			float splitDist = cascadeSplits[i];
			float splitStart = nearClip + lastSplitDist;
			float splitEnd = nearClip + splitDist;

			lastSplitDist = splitDist;

			float Hnear = 2 * tan(fov / 2) * splitStart;
			float Wnear = Hnear * aspectRatio;
			float Hfar = 2 * tan(fov / 2) * splitEnd;
			float Wfar = Hfar * aspectRatio;

			glm::vec3 centerFar = camPos + camForw * splitStart;
			glm::vec3 topLeftFar = centerFar + (camUp * Hfar / 2.0f) - (camRight * Wfar / 2.0f);
			glm::vec3 topRightFar = centerFar + (camUp * Hfar / 2.0f) + (camRight * Wfar / 2.0f);
			glm::vec3 bottomLeftFar = centerFar - (camUp * Hfar / 2.0f) - (camRight * Wfar / 2.0f);
			glm::vec3 bottomRightFar = centerFar - (camUp * Hfar / 2.0f) + (camRight * Wfar / 2.0f);

			glm::vec3 centerNear = camPos + camForw * splitEnd;
			glm::vec3 topLeftNear = centerNear + (camUp * Hnear / 2.0f) - (camRight * Wnear / 2.0f);
			glm::vec3 topRightNear = centerNear + (camUp * Hnear / 2.0f) + (camRight * Wnear / 2.0f);
			glm::vec3 bottomLeftNear = centerNear - (camUp * Hnear / 2.0f) - (camRight * Wnear / 2.0f);
			glm::vec3 bottomRightNear = centerNear - (camUp * Hnear / 2.0f) + (camRight * Wnear / 2.0f);

			glm::vec3 frustumCenter = (centerFar - centerNear) * 0.5f;

			std::array<glm::vec3, 8> frustumBoundsLightSpace
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

			// find extent of the frustum to define an ortho matrix around
			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumBoundsLightSpace[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 max = glm::vec3(radius);
			glm::vec3 min = -max;

			float l = min.x;
			float r = max.x;
			float b = min.y;
			float t = max.y;
			// because max.z is positive and in NDC the positive z axis is 
			// towards us so need to set it as the near plane flipped same for min.z.
			float n = -max.z;
			float f = -min.z;

			// Store split distance and matrix in cascade
			glm::mat4 projMat = glm::ortho(l, r, b, t, n, f);
			cascades.viewProjMats[i] = projMat * viewMat;
			cascades.splitDepths[i] = splitEnd;
		}

		return cascades;
	}
}