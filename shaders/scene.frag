#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec4 fragPosView;

layout(location = 0) out vec4 outColor;

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

layout (binding = 1) uniform sampler2DArray shadowMap;

layout(push_constant) uniform Push {
	mat4 modelMat;
	mat4 normalMat;
} push;

// Auxiliar variables
vec3 surfaceNormal;
vec3 viewDirection;

float glossiness = 60.0; // higher values -> sharper highlight
float d0 = 1.0; // Distance at which the light intensity is defined (reference point for attenuation)

float fragRadialDepth = length(fragPosView);

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

///////////// Useful functions /////////////

// Returns a value between 0.0 and 1.0 (0.0 = Fully in shadow, 1.0 = Fully out of shadow)
float shadowFunc(vec4 fragPosLS, vec2 offset, uint cascadeIndex)
{
	fragPosLS = fragPosLS / fragPosLS.w;
	
	float shadow = 1.0f;
	if (fragPosLS.z > -1.0f && fragPosLS.z < 1.0f)
	{
		float dist = texture(shadowMap, vec3(fragPosLS.st + offset, cascadeIndex)).r;
		if (fragPosLS.w > 0.0f && dist < fragPosLS.z)
		{
			shadow = 0.0f;
		}
	}
	return shadow;
}

// Same as above but with filtering applied for smoother shadows
float shadowFuncPCF(vec4 fragPosLS, uint cascadeIndex)
{
	ivec2 texDim = textureSize(shadowMap, 0).xy;
	float scale = 1.0;
	float dx = scale / float(texDim.x);
	float dy = scale / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += shadowFunc(fragPosLS, vec2(dx*x, dy*y), cascadeIndex);
			count++;
		}
	
	}
	return shadowFactor / count;
}

// Window function, to avoid a sharp cutoff at the boundary of the light's influence area
float winFunc(float fallOffDist, float maxFallOffDist)
{
	return pow(max(0, 1.0 - pow(fallOffDist/maxFallOffDist, 4)), 2);
}

// Inverse-square light attenuation function (lightly modified)
float falloffFunc(float squaredDistance, float lightRadius)
{
	return (d0 + lightRadius) / max(squaredDistance, lightRadius);
}

float attenuation(float squaredDistance, float lightRadius)
{
	// lightInfluenceRadius should be an exposed value for the user to tweak per light
	float lightInfluenceRadius = lightRadius * 100.0f;

	float fallOff = falloffFunc(squaredDistance, lightRadius);
	return winFunc(squaredDistance, lightInfluenceRadius) * fallOff;
}

vec3 pointLightShade(Light light)
{
	vec3 shadeColor = vec3(0.0);

	if(length(light.color.xyz) <= 0 || light.color.w == 0)
		return shadeColor;

	vec3 lightVec = light.position.xyz - fragPosWorld;
	vec3 lightDir = normalize(lightVec);

	vec3 lightIntensity = light.color.xyz * light.color.w;
	
	/// Diffuse
	float cosAngIncidence = max(dot(surfaceNormal, lightDir), 0);

	shadeColor += lightIntensity * cosAngIncidence;

	/// Specular
	vec3 halfAngle = normalize(lightDir + viewDirection);
	float blinnTerm = dot(surfaceNormal, halfAngle);
	blinnTerm = pow(max(blinnTerm, 0), glossiness);

	shadeColor += lightIntensity * blinnTerm;

	return shadeColor * attenuation(dot(lightVec, lightVec), light.radius);
}

vec3 directionalLightShade(Light light)
{
	vec3 shadeColor = vec3(0.0);

	if(length(light.color.xyz) <= 0 || light.color.w == 0)
		return shadeColor;

	// light.position is actually the direction vector
	vec3 lightDir = normalize(-light.position.xyz);

	vec3 lightIntensity = light.color.xyz * light.color.w;

	/// Diffuse
	float cosAngIncidence = max(dot(surfaceNormal, lightDir), 0);

	shadeColor += lightIntensity * cosAngIncidence;

	/// Specular
	vec3 halfAngle = normalize(lightDir + viewDirection);
	float blinnTerm = dot(surfaceNormal, halfAngle);
	blinnTerm = pow(max(blinnTerm, 0), glossiness);

	shadeColor += lightIntensity * blinnTerm;

	/// SHADOWS
	// Get cascade index for the current fragment's view position
	uint cascadeIndex = 0;
	for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if(fragRadialDepth > ubo.cascadeSplits[i]) {
			cascadeIndex = i + 1;
		}
	}

	// Apply shadowmap
	vec4 fragPosLightSpace = (biasMat * ubo.lightSpaceMats[cascadeIndex]) * vec4(fragPosWorld, 1.0);
	shadeColor *= shadowFuncPCF(fragPosLightSpace, cascadeIndex);

	return shadeColor;
}

vec3 shade()
{
	vec3 shadeColor = vec3(0.0);
	vec3 camPosWorld = ubo.invViewMat[3].xyz;
	viewDirection = normalize(camPosWorld - fragPosWorld);

	////// Ambient light //////
	shadeColor += ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

	////// Emissive light //////
	//shadeColor += Ke;

	////// Lights //////
	for (int i = 0; i < ubo.numLights; i++)
	{
		// Directional lights
		if (ubo.lights[i].type == 0)
			shadeColor += directionalLightShade(ubo.lights[i]);

		// Point lights
		if (ubo.lights[i].type == 1)
			shadeColor += pointLightShade(ubo.lights[i]);
	}

	return shadeColor;
}

void main() {
	surfaceNormal = normalize(fragNormalWorld);

	outColor = vec4(fragColor * shade(), 1.0);

	// Debugging
	//if(fragRadialDepth < ubo.cascadeSplits[0]) {
	//	outColor *= vec4(1.0, 0.0, 0.0, 1.0);
	//} else if(fragRadialDepth < ubo.cascadeSplits[1]) {
	//	outColor *= vec4(1.0, 1.0, 0.0, 1.0);
	//} else if(fragRadialDepth < ubo.cascadeSplits[2]) {
	//	outColor *= vec4(0.0, 1.0, 0.0, 1.0);
	//} else if(fragRadialDepth < ubo.cascadeSplits[3]) {
	//	outColor *= vec4(0.0, 0.0, 1.0, 1.0);
	//}
}