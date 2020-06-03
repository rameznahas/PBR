#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

// size: 176 bytes
layout (std140, binding = 0) uniform transformations {		// base alignment	// aligned offset
	mat4 M;													// 64 bytes			// 0
	mat4 MVP;												// 64 bytes			// 64
	mat3 normalMatrix;										// 48 bytes			// 128
};

out VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 uv;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vsOut.position = vec3(M * pos);
	vsOut.normal = normalize(normalMatrix * normal);
	vsOut.uv = uv;
}