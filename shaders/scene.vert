#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec4 fragPosLightSpace;

struct Light {
	int type;
	vec4 position; // ignore w
	vec4 color; // w is intensity
	float radius;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 viewMat;
	mat4 invViewMat;
	mat4 projMat;
	mat4 depthBiasMat;
	vec4 ambientLightColor; // w is intensity
	Light lights[10];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMat;
	mat4 normalMat;
} push;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
	vec4 positionWorld = push.modelMat * vec4(position, 1.0);

	fragColor = color;
	fragPosWorld = positionWorld.xyz;
	fragNormalWorld = normalize(mat3(push.normalMat) * normal);

	// For shadowmapping
	fragPosLightSpace = biasMat * ubo.depthBiasMat * vec4(fragPosWorld, 1.0);

	gl_Position = ubo.projMat * ubo.viewMat * vec4(fragPosWorld, 1.0);
}