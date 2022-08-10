#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 viewMat;
	mat4 invViewMat;
	mat4 projMat;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMat;
	mat4 normalMat;
} push;

void main() {
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);

	vec3 camPosWorld = ubo.invViewMat[3].xyz;
	vec3 viewDirection = normalize(camPosWorld - fragPosWorld);

	for (int i = 0; i < ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		vec3 lightDir = fragPosWorld - light.position.xyz;
		float attenuation = 1.0 / dot(lightDir, lightDir); // distance squared
		lightDir = normalize(lightDir);

		float cosAngIncidence = max(dot(surfaceNormal, -lightDir), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngIncidence;

		// specular lighting
		vec3 halfAngle = normalize(viewDirection - lightDir);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
		specularLight += intensity * blinnTerm;
	}

	outColor = vec4(fragColor * (diffuseLight + specularLight), 1.0);
}