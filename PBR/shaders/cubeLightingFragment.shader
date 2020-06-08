#version 430 core
#define NB_SAMPLES 20
#define NB_POINT_LIGHTS 6

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

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 uv;
} fsIn;

// size: 496 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLight[NB_POINT_LIGHTS];			// 480 bytes		// 0
	vec3 viewPos;									//  12 bytes		// 480
	float farPlane;									//   4 bytes		// 492
};

uniform sampler2D diffuseMap;
uniform samplerCube pointShadow[NB_POINT_LIGHTS];

out vec4 color;

// array of offset direction for sampling
vec3 gridSamplingDisk[NB_SAMPLES] = {
	vec3(1.0f, 1.0f,  1.0f), vec3(1.0f, -1.0f,  1.0f), vec3(-1.0f, -1.0f,  1.0f), vec3(-1.0f, 1.0f,  1.0f),
	vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f),
	vec3(1.0f, 1.0f,  0.0f), vec3(1.0f, -1.0f,  0.0f), vec3(-1.0f, -1.0f,  0.0f), vec3(-1.0f, 1.0f,  0.0f),
	vec3(1.0f, 0.0f,  1.0f), vec3(-1.0f, 0.0f,  1.0f), vec3(1.0f,   0.0f, -1.0f), vec3(-1.0f, 0.0f, -1.0f),
	vec3(0.0f, 1.0f,  1.0f), vec3(0.0f, -1.0f,  1.0f), vec3(0.0f,  -1.0f, -1.0f), vec3(0.0f,  1.0f, -1.0f)
};

float attenuation(Light light, vec3 fragToLight);
float shadow(vec3 fragToView, unsigned int idx);

void main() {
	color = vec4(0.0f);

	vec3 fragToView = viewPos - fsIn.position;
	vec3 diff = texture(diffuseMap, fsIn.uv).rgb;

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Light light = pointLight[i].light;
		vec3 fragToLight = pointLight[i].position - fsIn.position;

		vec3 ambient = light.ambient * diff;

		vec3 lightDir = normalize(fragToLight);
		float diffAngle = max(dot(lightDir, fsIn.normal), 0.0f);
		vec3 diffuse = light.color * diffAngle * diff;

		vec3 viewDir = normalize(fragToView);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float specAngle = pow(max(dot(halfwayDir, fsIn.normal), 0.0f), 64.0f);
		vec3 specular = light.color * specAngle * diff;
		color += vec4((ambient + (diffuse + specular) * shadow(fragToView, i)) * attenuation(light, fragToLight), 1.0f);
	}
}

float attenuation(Light light, vec3 fragToLight) {
	float distance = length(fragToLight);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2.0f));
}

float shadow(vec3 fragToView, unsigned int idx) {
	float shadow = 0.0f;
	float bias = 0.2f;
	float viewDistance = length(fragToView);
	float diskRadius = (1.0f + (viewDistance / farPlane)) / 25.0f;

	vec3 lightToFrag = fsIn.position - pointLight[idx].position;
	float currentDepth = length(lightToFrag);

	for (unsigned int i = 0; i < NB_SAMPLES; ++i) {
		float closestDepth = texture(pointShadow[idx], lightToFrag + gridSamplingDisk[i] * diskRadius).r * farPlane;
		shadow += currentDepth - bias < closestDepth ? 1.0f : -0.1f;
	}
	return shadow / NB_SAMPLES;
}