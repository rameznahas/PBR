#version 430 core
#define NB_POINT_LIGHTS 4

// size: 2 * vec4 = 32 bytes
struct PointLight {
	vec3 position;
	vec3 color;
};

struct Material {
	sampler2D albedo;
	sampler2D normal;
	sampler2D metallic;
	sampler2D roughness;
	sampler2D ao;
};

in VS_OUT {
	vec3 wPos;
	vec3 wNorm;
	vec2 uv;
} fsIn;

// size: 144 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLights[NB_POINT_LIGHTS];		// 128 bytes		// 0
	vec3 wViewPos;									//  16 bytes		// 128
};

// size: 4 bytes
layout (std140, binding = 2) uniform toneMapping {	// base alignment	// aligned offset
	float exposure;									// 4 bytes			// 0 
};

uniform Material material1;
uniform samplerCube irradianceMap;
uniform samplerCube preFilteredMap;
uniform sampler2D brdfIntegrationMap;

const float PI = 3.14159265359f;
const vec3 gammaCorrection = vec3(1.0f / 2.2f);
const float MAX_REFLECTION_LOD = 4.0f;

out vec4 color;

float NDF(float NdotH, float a2);
vec3 F(float VdotH, vec3 F0);
vec3 fRoughness(float VdotH, vec3 F0, float roughness);
float G(vec3 N, vec3 L, vec3 V, float k);
float geometrySchlickGGX(float NdotX, float k);
vec3 getWorldNorm();

void main() {
	vec3 albedo = texture(material1.albedo, fsIn.uv).rgb;
	vec3 N = getWorldNorm();
	float metallic = texture(material1.metallic, fsIn.uv).r;
	float roughness = texture(material1.roughness, fsIn.uv).r;
	float ao = texture(material1.ao, fsIn.uv).r;

	vec3 V = normalize(wViewPos - fsIn.wPos);
	float NdotV = max(dot(N, V), 0.0f);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	// lighting looks more correct squaring the roughness for NDF
	float a = roughness * roughness;
	// to be used in NDF
	float a2 = a * a;

	// remapping of roughness for direct lighting to be used in G
	float k = pow(roughness + 1.0f, 2) / 8.0f;

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
		kd *= (1.0f - metallic);
		vec3 diffuse = kd * albedo / PI;

		// specular part
		float NdotL = max(dot(N, L), 0.0f);
		vec3 specular =
			NDF(max(dot(N, H), 0.0f), a2) *
			f *
			G(N, L, V, k);

		specular /= max((4.0f * NdotV * NdotL), 0.001f); // prevent division by 0

		L0 += (diffuse + specular) * radiance * NdotL;
	}

	vec3 ks = fRoughness(NdotV, F0, roughness);

	vec3 kd = vec3(1.0f) - ks;
	kd *= (1.0f - metallic);
	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = kd * irradiance * albedo;

	vec3 R = reflect(-V, N);
	vec3 preFilteredColor = textureLod(preFilteredMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
	vec3 specular = preFilteredColor * (ks * brdf.x + brdf.y);

	vec3 ambient = (diffuse + specular) * ao;

	vec3 HDRcolor = ambient + L0;
	vec3 toneMapping = vec3(1.0f) - exp(-HDRcolor * exposure);

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

vec3 fRoughness(float VdotH, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - VdotH, 5.0);
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

vec3 getWorldNorm() {
	vec3 tNorm = texture(material1.normal, fsIn.uv).xyz * 2.0f - 1.0f;

	vec3 Q1 = dFdx(fsIn.wPos);
	vec3 Q2 = dFdy(fsIn.wPos);
	vec2 st1 = dFdx(fsIn.uv);
	vec2 st2 = dFdy(fsIn.uv);

	vec3 N = normalize(fsIn.wNorm);
	vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tNorm);
}