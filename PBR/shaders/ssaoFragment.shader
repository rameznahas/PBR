#version 430 core
#define NB_SSAO_SAMPLES 64

in VS_OUT {
	vec2 uv;
} fsIn;

uniform sampler2D gVSpositions;
uniform sampler2D gVSnormals;
uniform sampler2D noise;
uniform vec3 kernel[NB_SSAO_SAMPLES];
uniform mat4 P;
uniform vec2 screenDim;
uniform float noiseTexRes;
uniform int ssaoPower;
uniform float radius;

const vec2 noiseScale = screenDim / noiseTexRes;
const float bias = 0.025f;

out float color;

void main() {
	vec3 vsPos = texture(gVSpositions, fsIn.uv).xyz;
	vec3 vsNorm = texture(gVSnormals, fsIn.uv).rgb;
	vec3 randomVec = texture(noise, fsIn.uv * noiseScale).xyz;

	vec3 T = normalize(randomVec - dot(randomVec, vsNorm) * vsNorm);
	vec3 B = cross(vsNorm, T);
	mat3 vsTBN = mat3(T, B, vsNorm);

	float occlusion = 0.0f;
	for (unsigned int i = 0; i < NB_SSAO_SAMPLES; ++i) {
		vec3 samplePos = vsTBN * kernel[i]; // from tangent to view space
		samplePos = vsPos + samplePos * radius;

		vec4 ssSamplePos = P * vec4(samplePos, 1.0f);		// to screen space
		ssSamplePos.xyz /= ssSamplePos.w;					// perspective divide
		ssSamplePos.xyz = ssSamplePos.xyz * 0.5f + 0.5f;	// to range [0-1] so we can use it to sample gVSposition

		float sampleDepth = texture(gVSpositions, ssSamplePos.xy).z;
		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(vsPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0f : 0.0f) * rangeCheck;
	}
	occlusion = 1.0f - occlusion / NB_SSAO_SAMPLES;
	color = pow(occlusion, ssaoPower);
}