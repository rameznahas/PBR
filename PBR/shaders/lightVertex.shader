#version 430 core

layout (location = 0) in vec3 position;

// size: 240 bytes
layout (std140, binding = 0) uniform transforms {	// base alignment	// aligned offset
	mat4 M;											// 64 bytes			// 0
	mat4 V;											// 64 bytes			// 64
	mat4 MVP;										// 64 bytes			// 128
	mat3 normalMatrix;								// 48 bytes			// 192
};

void main() {
	gl_Position = MVP * vec4(position, 1.0f);
}