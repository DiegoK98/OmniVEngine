#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec4 fragPosLightSpace;

layout(location = 0) out vec4 outColor;

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

layout (binding = 1) uniform sampler2D shadowMap;

layout(push_constant) uniform Push {
	mat4 modelMat;
	mat4 normalMat;
} push;

// Auxiliar variables
vec3 surfaceNormal;
vec3 viewDirection;

float glossiness = 60.0; // higher values -> sharper highlight
float d0 = 1.0; // Distance at which the light intensity is defined (reference point for attenuation)

///////////// Useful functions /////////////

// Returns a value between 0.0 and 1.0 (0.0 = Fully in shadow, 1.0 = Fully out of shadow)
float shadowFunc(vec4 fragPosLS, vec2 offset)
{
	fragPosLS = fragPosLS / fragPosLS.w;
	
	float shadow = 1.0f;
	if (fragPosLS.z > -1.0f && fragPosLS.z < 1.0f)
	{
		float dist = texture(shadowMap, fragPosLS.st + offset).r;
		if (fragPosLS.w > 0.0f && dist < fragPosLS.z)
		{
			shadow = 0.0f;
		}
	}
	return shadow;
}

// Same as above but with filtering applied for smoother shadows
float shadowFuncPCF(vec4 fragPosLS)
{
	fragPosLS = fragPosLS / fragPosLS.w;

	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.0;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += shadowFunc(fragPosLS, vec2(dx*x, dy*y));
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
	
	// Diffuse
	float cosAngIncidence = max(dot(surfaceNormal, lightDir), 0);

	shadeColor += lightIntensity * cosAngIncidence;

	// Specular
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

	vec3 lightDir = normalize(-light.position.xyz);

	vec3 lightIntensity = light.color.xyz * light.color.w;

	// Diffuse
	float cosAngIncidence = max(dot(surfaceNormal, lightDir), 0);

	shadeColor += lightIntensity * cosAngIncidence;

	// Specular
	vec3 halfAngle = normalize(lightDir + viewDirection);
	float blinnTerm = dot(surfaceNormal, halfAngle);
	blinnTerm = pow(max(blinnTerm, 0), glossiness);

	shadeColor += lightIntensity * blinnTerm;

	// Apply shadowmapping
	shadeColor *= shadowFuncPCF(fragPosLightSpace);

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
}