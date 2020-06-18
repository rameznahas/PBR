#version 430 core

in vec3 uvw;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359f;
const float fullCircle = 2.0f * PI;

out vec4 color;

float radicalInverseVDC(uint bits);
vec2 hammersley(uint i, uint N);
vec3 importanceSamplingGGX(vec2 xi, vec3 T, vec3 B, vec3 N, float a2);
float NDF(float NdotH, float a2);

void main() {
	vec3 N = normalize(uvw);
	vec3 R = N;
	vec3 V = R;

	// construct T, B from N to later transform tangent-space vectors to world-space
	vec3 wUp = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
	vec3 T = normalize(cross(wUp, N));
	vec3 B = cross(N, T);

	// lighting looks more correct squaring the roughness for NDF
	float a = roughness * roughness;
	float a2 = a * a;

	const float RES = textureSize(environmentMap, 0).x;
	float saTexel = 4.0f * PI / (6.0f * RES * RES);

	const uint NB_SAMPLES = 1024u;
	float totalWeight = 0.0f;
	vec3 preFilteredColor = vec3(0.0f);
	for (uint i = 0u; i < NB_SAMPLES; ++i) {
		vec2 xi = hammersley(i, NB_SAMPLES);
		// halfway vector
		vec3 H = importanceSamplingGGX(xi, T, B, N, a2);
		vec3 sampleVec = normalize(2.0f * dot(V, H) * H - V);

		float NdotL = max(dot(N, sampleVec), 0.0f);
		if (NdotL > 0.0f) {
			// sample from the env.'s mipmap level, based on roughness / pdf
			float NdotH = max(dot(N, H), 0.0f);
			float VdotH = max(dot(V, H), 0.0f);
			float D = NDF(NdotH, a2);
			float pdf = D * NdotH / (4.0f * VdotH) + 0.0001f;
			float saSample = 1.0f / (float(NB_SAMPLES) * pdf + 0.0001f);

			float mipmapLevel = roughness == 0.0f ? 0.0f : (0.5f * log2(saSample / saTexel));

			preFilteredColor += textureLod(environmentMap, sampleVec, mipmapLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	preFilteredColor /= totalWeight;

	color = vec4(preFilteredColor, 1.0f);
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

// Normal Distribution Function using TR-GGX
float NDF(float NdotH, float a2) {
	return a2 / (PI * pow((NdotH * NdotH) * (a2 - 1.0f) + 1.0f, 2));
}