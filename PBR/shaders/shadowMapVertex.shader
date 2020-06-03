#version 430 core

layout (location = 0) in vec3 position;

// size: 64 bytes
layout (std140, binding = 2) uniform shadow {		// base alignement	// aligned offset
	mat4 MVP;										// 64 bytes			// 0
};

void main() {
	gl_Position = MVP * vec4(position, 1.0f);
}