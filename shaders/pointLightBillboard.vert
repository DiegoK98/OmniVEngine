#version 450

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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
	vec4 ambientLightColor; // w is intensity
	Light lights[10];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

void main() {
	// Offset from center of pointLight to current vertex (disregarding light radius, so normalized)
	fragOffset = OFFSETS[gl_VertexIndex];

	vec3 cameraRightWorld = { ubo.viewMat[0][0], ubo.viewMat[1][0], ubo.viewMat[2][0] };
	vec3 cameraUpWorld = { ubo.viewMat[0][1], ubo.viewMat[1][1], ubo.viewMat[2][1] };

	vec3 positionWorld = push.position.xyz + push.radius * fragOffset.x * cameraRightWorld + push.radius * fragOffset.y * cameraUpWorld;

	gl_Position = ubo.projMat * ubo.viewMat * vec4(positionWorld, 1.0);
}