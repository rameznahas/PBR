#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

// size: 176 bytes
layout (std140, binding = 0) uniform transforms {	// base alignment	// aligned offset
	mat4 M;											// 64 bytes			// 0
	mat4 MVP;										// 64 bytes			// 64
	mat3 normalMatrix;								// 48 bytes			// 128
};

out VS_OUT {
	vec3 worldPos;
	vec2 uv;
	mat3 TBN;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vsOut.worldPos = vec3(M * pos);
	vsOut.uv = uv;

	vec3 T = normalize(normalMatrix * tangent);
	vec3 N = normalize(normalMatrix * normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	vsOut.TBN = mat3(T, B, N);
}