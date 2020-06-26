#version 430 core

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

out VS_OUT {
	vec3 wPos;
	vec3 wNorm;
	vec2 uv;
	mat3 TBN;
	mat3 invTBN;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vec3 wNorm = NM * normal;

	vec3 T = normalize(NM * tangent);
	vec3 N = normalize(wNorm);
	//T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	vsOut.wPos = vec3(M * pos);
	vsOut.wNorm = wNorm;
	vsOut.uv = uv;
	vsOut.TBN = mat3(T, B, N);
	vsOut.invTBN = transpose(vsOut.TBN);
}