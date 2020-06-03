#version 430 core

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
	vec4 lightSpacePosition;
} fsIn;

// size: 96 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLight;							// 80 bytes			// 0
	vec3 viewPos;									// 16 bytes			// 80
};

uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform int samples;

out vec4 color;

float shadow(vec3 lightDir);

void main() {
	vec3 diff = texture(tex, fsIn.uv).rgb;

	Light light = pointLight.light;

	vec3 ambient = light.ambient * diff;

	vec3 lightDir = normalize(pointLight.position - fsIn.position);
	float diffAngle = max(dot(lightDir, fsIn.normal), 0.0f);
	vec3 diffuse = light.color * diffAngle;

	vec3 viewDir = normalize(viewPos - fsIn.position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specAngle = pow(max(dot(halfwayDir, fsIn.normal), 0.0f), 64.0f);
	vec3 specular = light.color * specAngle;

	color = vec4((ambient + (diffuse + specular) * shadow(lightDir)) * diff, 1.0f);
}

float shadow(vec3 lightDir) {
	vec3 ndc = fsIn.lightSpacePosition.xyz / fsIn.lightSpacePosition.w;
	vec3 depthRange = 0.5f * ndc + 0.5f;

	if (depthRange.z > 1.0f)
		return 1.0f;

	float bias = max(0.05f * (1.0f - dot(fsIn.normal, lightDir)), 0.005f);
	
	float shadow = 0.0f;
	vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
	int range = samples / 2;
	for (int x = -range; x <= range; ++x) {
		for (int y = -range; y <= range; ++y) {
			float pcfDepth = texture(shadowMap, depthRange.xy + vec2(x, y) * texelSize).r;
			shadow += depthRange.z - bias > pcfDepth ? 0.0f : 1.0f;
		}
	}

	return shadow / pow(samples, 2);
}