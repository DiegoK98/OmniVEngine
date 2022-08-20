#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
	float radius;
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

// Auxiliar variables
vec3 surfaceNormal;
vec3 viewDirection;

float glossiness = 60.0; // 512.0 was the value from the LveEngine tutorial
float d0 = 1.0; // Distance at which the light intensity is defined (reference point for attenuation)

float attenuation(float squaredDistance, float lightRadius)
{
	return (d0 + lightRadius) / max(squaredDistance, lightRadius); // Inverse-square light attenuation. Added my own modification
}

vec3 pointLightShade(PointLight light)
{
	vec3 shadeColor = vec3(0.0);

	if(length(light.color.xyz) <= 0 || light.color.w == 0)
		return shadeColor;

	vec3 lightVec = fragPosWorld - light.position.xyz;
	vec3 lightDir = normalize(lightVec);
	
	// Diffuse
	float cosAngIncidence = max(dot(surfaceNormal, -lightDir), 0);
	vec3 intensity = light.color.xyz * light.color.w;

	shadeColor += intensity * cosAngIncidence;

	// Specular
	vec3 halfAngle = normalize(viewDirection - lightDir);
	float blinnTerm = dot(surfaceNormal, halfAngle);
	blinnTerm = clamp(blinnTerm, 0, 1);
	blinnTerm = pow(blinnTerm, glossiness); // higher values -> sharper highlight

	shadeColor += intensity * blinnTerm;

	return shadeColor * attenuation(dot(lightVec, lightVec), light.radius);
}

vec3 shade() 
{
	vec3 shadeColor = vec3(0.0);
	vec3 camPosWorld = ubo.invViewMat[3].xyz;
	viewDirection = normalize(camPosWorld - fragPosWorld);

	////// Ambient //////
	shadeColor += ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

	////// Point lights //////
	for (int i = 0; i < ubo.numLights; i++)
	{
		shadeColor += pointLightShade(ubo.pointLights[i]);
	}

	////// Directionals //////
	//shadeColor += directionalLightShade(directionalLight1Intensity, directionalLight1Direction);

	////// Focal lights //////
	//shadeColor += focalLightShade(focalLight1Intensity, focalLight1Position, focalLight1Direction, focalLight1CutOff, focalLight1FocalDiff);

	////// Emissive //////
	//shadeColor += Ke;

	return shadeColor;
}

void main() {
	surfaceNormal = normalize(fragNormalWorld);

	outColor = vec4(fragColor * shade(), 1.0);
}