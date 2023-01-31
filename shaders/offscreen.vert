#version 450

layout(location = 0) in vec3 position;

// todo: pass via specialization constant
#define SHADOW_MAP_CASCADE_COUNT 4
#define MAX_LIGHTS 10

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
	mat4 lightSpaceMats[SHADOW_MAP_CASCADE_COUNT];
	vec4 cascadeSplits;
	vec4 ambientLightColor; // w is intensity
	Light lights[MAX_LIGHTS];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMat;
	mat4 normalMat;
	uint cascadeIndex;
} push;

void main() {
	gl_Position = ubo.lightSpaceMats[push.cascadeIndex] * push.modelMat * vec4(position, 1.0);
}