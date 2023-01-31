#version 450

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

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
	vec4 ambientLightColor; // w is intensity
	Light lights[MAX_LIGHTS];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

const float M_PI = 3.1415926538;

void main() {
	// fragOffset is normalized, so that we don't need to account for light radius
	float dis = sqrt(dot(fragOffset, fragOffset));

	if (dis >= 1.0) {
		discard;
	}

	float cosDis = 0.5 * (cos(dis * M_PI) + 1.0); // ranges from 1 -> 0

	outColor = vec4(push.color.xyz + 0.5 * cosDis, cosDis);
}