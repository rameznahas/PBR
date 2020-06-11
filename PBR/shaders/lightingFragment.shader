#version 430 core
#define NB_SAMPLES 20
#define NB_POINT_LIGHTS 9

struct Light {
	vec3 color;
	float kc;
	vec3 ambient;
	float kl;
	vec3 diffuse;
	float kq;
	vec3 specular;
};

struct PointLight {
	Light light;
	vec3 position;
};

struct Material {
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
};

in VS_OUT {
	vec3 worldPos;
	vec3 tangentPos;
	vec3 tangentViewPos;
	vec3 tangentLightPos[NB_POINT_LIGHTS];
	vec2 uv;
} fsIn;

// size: 736 bytes
layout(std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLight[NB_POINT_LIGHTS];			// 720 bytes		// 0
	vec3 worldViewPos;								//  12 bytes		// 720
	float farPlane;									//   4 bytes		// 732
};

uniform Material material1;
uniform samplerCube pointShadow[NB_POINT_LIGHTS];

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bloomColor;

// array of offset direction for sampling
vec3 gridSamplingDisk[NB_SAMPLES] = {
	vec3(1.0f, 1.0f,  1.0f), vec3(1.0f, -1.0f,  1.0f), vec3(-1.0f, -1.0f,  1.0f), vec3(-1.0f, 1.0f,  1.0f),
	vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f),
	vec3(1.0f, 1.0f,  0.0f), vec3(1.0f, -1.0f,  0.0f), vec3(-1.0f, -1.0f,  0.0f), vec3(-1.0f, 1.0f,  0.0f),
	vec3(1.0f, 0.0f,  1.0f), vec3(-1.0f, 0.0f,  1.0f), vec3(1.0f,   0.0f, -1.0f), vec3(-1.0f, 0.0f, -1.0f),
	vec3(0.0f, 1.0f,  1.0f), vec3(0.0f, -1.0f,  1.0f), vec3(0.0f,  -1.0f, -1.0f), vec3(0.0f,  1.0f, -1.0f)
};

float attenuation(Light light, vec3 tangentFragToLight);
float shadow(vec3 worldFragToView, unsigned int idx);

void main() {
	color = vec4(0.0f);

	vec3 worldFragToView = worldViewPos - fsIn.worldPos;
	vec3 viewDir = normalize(fsIn.tangentViewPos - fsIn.tangentPos);

	vec3 diff = texture(material1.diffuseMap, fsIn.uv).rgb;
	float spec = texture(material1.specularMap, fsIn.uv).r;
	vec3 normal = texture(material1.normalMap, fsIn.uv).rgb;	// in range [0-1]
	normal = normalize(normal * 2.0f - 1.0f);		// to range [-1, 1] before normalizing

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Light light = pointLight[i].light;

		vec3 tangentFragToLight = fsIn.tangentLightPos[i] - fsIn.tangentPos;

		vec3 ambient = light.ambient * diff;

		vec3 lightDir = normalize(tangentFragToLight);
		float diffAngle = max(dot(lightDir, normal), 0.0f);
		vec3 diffuse = light.color * diffAngle * diff;

		vec3 halfwayDir = normalize(lightDir + viewDir);
		float specAngle = pow(max(dot(halfwayDir, normal), 0.0f), 32.0f);
		vec3 specular = light.color * specAngle * spec;
		color += vec4((ambient + (diffuse + specular) * shadow(worldFragToView, i)) * attenuation(light, tangentFragToLight), 1.0f);
	}

	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	bloomColor = brightness > 2.5f ? color : vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

float attenuation(Light light, vec3 tangentFragToLight) {
	float distance = length(tangentFragToLight);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2.0f));
}

float shadow(vec3 worldFragToView, unsigned int idx) {
	float shadow = 0.0f;
	float bias = 0.2f;
	float viewDistance = length(worldFragToView);
	float diskRadius = (1.0f + (viewDistance / farPlane)) / 25.0f;

	vec3 lightToFrag = fsIn.worldPos - pointLight[idx].position;
	float currentDepth = length(lightToFrag);

	for (unsigned int i = 0; i < NB_SAMPLES; ++i) {
		float closestDepth = texture(pointShadow[idx], lightToFrag + gridSamplingDisk[i] * diskRadius).r * farPlane;
		shadow += currentDepth - bias < closestDepth ? 1.0f : -0.1f;
	}
	return shadow / NB_SAMPLES;
}