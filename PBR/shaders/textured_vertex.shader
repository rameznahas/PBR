#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 M;
uniform mat4 MVP;
uniform mat3 normal_matrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec2 UV;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vertex_position = vec3(M * pos);
	vertex_normal = normalize(normal_matrix * normal);
	UV = uv;
}