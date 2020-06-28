#version 430 core
#define NB_POINT_LIGHTS 4

// size: 2 * vec4 = 32 bytes
struct PointLight {
	vec3 position;
	vec3 color;
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 uv;

// size: 176 bytes
layout (std140, binding = 0) uniform transforms {		// base alignement	// aligned offset
	mat4 M;												// 64 bytes			// 0
	mat4 MVP;											// 64 bytes			// 64
	mat3 NM;											// 48 bytes			// 128
};

// size: 144 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLights[NB_POINT_LIGHTS];		// 128 bytes		// 0
	vec3 wViewPos;									//  16 bytes		// 128
};

out VS_OUT {
	vec3 wPos;
	vec3 wNorm;
	vec2 uv;
	mat3 TBN;
	vec3 tViewDir;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vec3 wNorm = NM * normal;

	vec3 T = normalize(NM * tangent);
	vec3 N = normalize(wNorm);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	vsOut.wPos = vec3(M * pos);
	vsOut.wNorm = wNorm;
	vsOut.uv = uv;
	vsOut.TBN = mat3(T, B, N);
	vsOut.tViewDir = transpose(vsOut.TBN) * (wViewPos - vsOut.wPos);
}