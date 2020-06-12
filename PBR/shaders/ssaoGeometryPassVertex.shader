#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

// size: 240 bytes
layout (std140, binding = 0) uniform transforms {	// base alignment	// aligned offset
	mat4 M;											// 64 bytes			// 0
	mat4 V;											// 64 bytes			// 64
	mat4 MVP;										// 64 bytes			// 128
	mat3 normalMatrix;								// 48 bytes			// 192
};

out VS_OUT {
	vec3 vsPos;
	vec2 uv;
	mat3 vsTBN;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vsOut.vsPos = vec3(V * M * pos);
	vsOut.uv = uv;

	mat3 vsProj = mat3(V) * normalMatrix;

	vec3 T = normalize(vsProj * tangent);
	vec3 N = normalize(vsProj * normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	vsOut.vsTBN = mat3(T, B, N);
}