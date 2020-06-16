#version 430 core
#define NB_POINT_LIGHTS 4

// size: 2 * vec4 = 32 bytes
struct PointLight {
	vec3 position;
	vec3 color;
};

struct Material {
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
};

in VS_OUT {
	vec3 wPos;
	vec3 wNorm;
	vec2 uv;
} fsIn;

// size: 144 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLights[NB_POINT_LIGHTS];		// 128 bytes		// 0
	vec3 wViewPos;									//  14 bytes		// 128
};

uniform Material material1;

const float PI = 3.14159265359f;
const vec3 gammaCorrection = vec3(1.0f / 2.2f);

//layout (location = 0) out vec3 color;
//layout (location = 1) out vec3 bloomColor;

out vec4 color;

float NDF(float NdotH, float a2);
vec3 F(float VdotH, vec3 F0);
float G(vec3 N, vec3 L, vec3 V, float k);
float geometrySchlickGGX(float NdotX, float k);

void main() {
	vec3 V = normalize(wViewPos - fsIn.wPos);
	vec3 N = normalize(fsIn.wNorm);
	float NdotV = max(dot(N, V), 0.0f);
	
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, material1.albedo, material1.metallic);

	// lighting looks more correct squaring the roughness for NDF
	float a = material1.roughness * material1.roughness;
	// to be used in NDF
	float a2 = a * a;

	// remapping of roughness for direct lighting to be used in G
	float k = pow(material1.roughness + 1.0f, 2) / 8.0f;

	vec3 L0 = vec3(0.0f);
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		PointLight light = pointLights[i];
		vec3 lightDir = light.position - fsIn.wPos;

		// per-light radiance
		vec3 L = normalize(lightDir);
		float distance = length(lightDir);
		float attenuation = 1.0f / (distance * distance);
		vec3 radiance = light.color * attenuation;
		
		// Cook-Torrance BRDF
		vec3 H = normalize(L + V);

		// diffuse part
		vec3 f = F(clamp(dot(V, H), 0.0f, 1.0f), F0);
		vec3 kd = vec3(1.0f) - f;
		// metallic surfaces don't refract light, so nullify
		// diffuse component in case of metallic surface
		kd *= (1.0f - material1.metallic);
		vec3 diffuse = kd * material1.albedo / PI;

		// specular part
		float NdotL = max(dot(N, L), 0.0f);
		vec3 specular =
			NDF(max(dot(N, H), 0.0f), a2) *
			f *
			G(N, L, V, k);

		specular /= max((4.0f * NdotV * NdotL), 0.001f); // prevent division by 0

		L0 += (diffuse + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03f) * material1.albedo * material1.ao;
	/*color = ambient + L0;

	float brightness = dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
	bloomColor = brightness > 5.0f ? color : vec3(0.0f);*/

	vec3 HDRcolor = ambient + L0;
	vec3 toneMapping = HDRcolor / (HDRcolor + vec3(1.0f));

	color = vec4(pow(toneMapping, gammaCorrection), 1.0f);
}

// Normal Distribution Function using TR-GGX
float NDF(float NdotH, float a2) {
	return a2 / (PI * pow((NdotH * NdotH) * (a2 - 1.0f) + 1.0f, 2));
}

// Freshnel-Schlick function for reflection percentage
vec3 F(float VdotH, vec3 F0) {
	return F0 + (1.0f - F0) * pow(1.0f - VdotH, 5);
}

// Geometry function using Smith's method
float G(vec3 N, vec3 L, vec3 V, float k) {
	float NdotL = max(dot(N, L), 0.0f);
	float NdotV = max(dot(N, V), 0.0f);

	return geometrySchlickGGX(NdotL, k) * geometrySchlickGGX(NdotV, k);
}

// Geometry function using Schlick-GGX
float geometrySchlickGGX(float NdotX, float k) {
	return NdotX / (NdotX * (1.0f - k) + k);
}