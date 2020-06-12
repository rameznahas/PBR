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

in VS_OUT {
	vec2 uv;
} fsIn;

// size: 736 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLight[NB_POINT_LIGHTS];			// 720 bytes		// 0
	vec3 worldViewPos;								//  12 bytes		// 720
	float farPlane;									//   4 bytes		// 732
};

uniform sampler2D gPositions;
uniform sampler2D gNormals;
uniform sampler2D gColorSpecs;
uniform sampler2D ssao;
uniform samplerCube pointShadow[NB_POINT_LIGHTS];
uniform mat4 V1;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bloomColor;

// array of offset direction for sampling
vec3 gridSamplingDisk[NB_SAMPLES] = {
	vec3(1.0f, 1.0f,  1.0f), vec3(1.0f, -1.0f,  1.0f), vec3(-1.0f, -1.0f,  1.0f), vec3(-1.0f, 1.0f,  1.0f),
	vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f),
	vec3(1.0f, 1.0f,  0.0f), vec3(1.0f, -1.0f,  0.0f), vec3(-1.0f, -1.0f,  0.0f), vec3(-1.0f, 1.0f,  0.0f),
	vec3(1.0f, 0.0f,  1.0f), vec3(-1.0f, 0.0f,  1.0f), vec3(1.0f,   0.0f, -1.0f), vec3(-1.0f, 0.0f, -1.0f),
	vec3(0.0f, 1.0f,  1.0f), vec3(0.0f, -1.0f,  1.0f), vec3(0.0f,  -1.0f, -1.0f), vec3(0.0f,  1.0f, -1.0f)
};

float attenuation(Light light, vec3 worldFragToLight);
float shadow(vec3 worldPos, float diskRadius, unsigned int idx);

void main() {
	color = vec4(0.0f);

	vec3 vsPos = texture(gPositions, fsIn.uv).rgb;
	vec3 worldPos = vec3(V1 * vec4(vsPos, 1.0f));
	vec3 vsNorm = texture(gNormals, fsIn.uv).rgb;
	vec3 worldNorm = vec3(V1 * vec4(vsNorm, 0.0f));
	vec3 diff = texture(gColorSpecs, fsIn.uv).rgb;
	float spec = texture(gColorSpecs, fsIn.uv).a;
	float ao = texture(ssao, fsIn.uv).r;
	vec3 worldFragToView = worldViewPos - worldPos;
	vec3 viewDir = normalize(worldFragToView);
	float diskRadius = (1.0f + (length(worldFragToView) / farPlane)) / 25.0f;

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Light light = pointLight[i].light;

		vec3 ambient = light.ambient * diff * ao;

		vec3 worldFragToLight = pointLight[i].position - worldPos;
		vec3 lightDir = normalize(worldFragToLight);
		float diffAngle = max(dot(lightDir, worldNorm), 0.0f);
		vec3 diffuse = light.color * diffAngle * diff;

		vec3 halfwayDir = normalize(lightDir + viewDir);
		float specAngle = pow(max(dot(halfwayDir, worldNorm), 0.0f), 32.0f);
		vec3 specular = light.color * specAngle * spec;
		color += vec4((ambient + (diffuse + specular) * shadow(worldPos, diskRadius, i)) * attenuation(light, worldFragToLight), 1.0f);
	}

	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	bloomColor = brightness > 2.5f ? color : vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

float attenuation(Light light, vec3 worldFragToLight) {
	float distance = length(worldFragToLight);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2.0f));
}

float shadow(vec3 worldPos, float diskRadius, unsigned int idx) {
	float shadow = 0.0f;
	float bias = 0.2f;

	vec3 lightToFrag = worldPos - pointLight[idx].position;
	float currentDepth = length(lightToFrag);

	for (unsigned int i = 0; i < NB_SAMPLES; ++i) {
		float closestDepth = texture(pointShadow[idx], lightToFrag + gridSamplingDisk[i] * diskRadius).r * farPlane;
		shadow += currentDepth - bias < closestDepth ? 1.0f : -0.1f;
	}
	return shadow / NB_SAMPLES;
}