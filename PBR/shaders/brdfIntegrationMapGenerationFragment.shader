#version 430 core

in vec2 UV;

const float PI = 3.14159265359f;
const float fullCircle = 2.0f * PI;

out vec2 color;

float radicalInverseVDC(uint bits);
vec2 hammersley(uint i, uint N);
vec3 importanceSamplingGGX(vec2 xi, vec3 T, vec3 B, vec3 N, float a2);
float G(vec3 N, vec3 L, vec3 V, float k);
float geometrySchlickGGX(float NdotX, float k);

void main() {
	float NdotV = UV.x;
	vec3 V = vec3(
		sqrt(1.0f - NdotV * NdotV),
		0.0f,
		NdotV
	);
	vec3 N = vec3(0.0f, 0.0f, 1.0f);

	// construct T, B from N to later transform tangent-space vectors to world-space
	vec3 wUp = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
	vec3 T = normalize(cross(wUp, N));
	vec3 B = cross(N, T);

	float roughness = UV.y;

	// lighting looks more correct squaring the roughness for NDF
	float a = roughness * roughness;
	float a2 = a * a;

	// remapping of roughness for IBL to be used in G
	float k = a / 2.0f;

	const uint NB_SAMPLES = 1024u;
	color = vec2(0.0f);
	for (uint i = 0u; i < NB_SAMPLES; ++i) {
		vec2 xi = hammersley(i, NB_SAMPLES);
		// halfway vector
		vec3 H = importanceSamplingGGX(xi, T, B, N, a2);
		vec3 sampleVec = normalize(2.0f * dot(V, H) * H - V);

		float NdotL = max(sampleVec.z, 0.0f);
		float NdotH = max(H.z, 0.0f);
		float VdotH = max(dot(V, H), 0.0f);

		if (NdotL > 0.0f) {
			float g = G(N, sampleVec, V, k);
			float g1 = (g * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0f - VdotH, 5.0f);
			color += vec2(
				(1.0f - Fc) * g1,
				Fc * g1
			);
		}
	}
	color /= float(NB_SAMPLES);
}

float radicalInverseVDC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N) {
	return vec2(float(i) / N, radicalInverseVDC(i));
}

vec3 importanceSamplingGGX(vec2 xi, vec3 T, vec3 B, vec3 N, float a2) {
	float phi = fullCircle * xi.x;
	float cosTheta = sqrt((1.0f - xi.y) / (1.0f + (a2 - 1.0f) * xi.y));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	// spherical coord. to cartesian (in tangent space)
	vec3 tangentH = vec3(
		cos(phi) * sinTheta,
		sin(phi) * sinTheta,
		cosTheta
	);

	// world-space halfway vector
	vec3 H = tangentH.x * T + tangentH.y * B + tangentH.z * N;
	return normalize(H);
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