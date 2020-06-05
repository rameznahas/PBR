#version 430 core
#define NB_SAMPLES 20
#define NB_POINT_LIGHTS 2

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

// size: 96 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLight;							// 80 bytes			// 0
	vec3 viewPos;									// 12 bytes			// 80
	float farPlane;									//  4 bytes			// 92
};

uniform sampler2D tex;
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
float shadow(vec3 fragToView);

void main() {
	Light light = pointLight.light;
	vec3 fragToLight = pointLight.position - fsIn.position;
	vec3 fragToView = viewPos - fsIn.position;

	vec3 diff = texture(tex, fsIn.uv).rgb;

	vec3 ambient = light.ambient * diff;

	vec3 lightDir = normalize(fragToLight);
	float diffAngle = max(dot(lightDir, fsIn.normal), 0.0f);
	vec3 diffuse = light.color * diffAngle * diff;

	vec3 viewDir = normalize(fragToView);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specAngle = pow(max(dot(halfwayDir, fsIn.normal), 0.0f), 64.0f);
	vec3 specular = light.color * specAngle * diff;
	
	color = vec4((ambient + (diffuse + specular) * shadow(fragToView)) * attenuation(light, fragToLight), 1.0f);
}

float attenuation(Light light, vec3 fragToLight) {
	float distance = length(fragToLight);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2.0f));
}

float shadow(vec3 fragToView) {
	float shadow = 0.0f;
	float bias = 0.05f;
	float viewDistance = length(fragToView);
	float diskRadius = (1.0f + (viewDistance / farPlane)) / 25.0f;

	vec3 lightToFrag = fsIn.position - pointLight.position;
	float currentDepth = length(lightToFrag);

	for (unsigned int i = 0; i < NB_SAMPLES; ++i) {
		float closestDepth = texture(pointShadow[0], lightToFrag + gridSamplingDisk[i] * diskRadius).r * farPlane;
		shadow += currentDepth - bias < closestDepth ? 1.0f : 0.0f;
	}
	return shadow / NB_SAMPLES;
}