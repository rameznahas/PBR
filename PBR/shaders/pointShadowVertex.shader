#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 M;
uniform mat4 MVP;

out vec3 fragPos;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	fragPos = vec3(M * pos);
}