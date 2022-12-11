#version 450

layout(location = 0) in vec3 position;

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

void main() {
	gl_Position = ubo.depthBiasMat * push.modelMat * vec4(position, 1.0);
}