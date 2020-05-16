#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

uniform mat4 M;
uniform mat4 MVP;
uniform mat3 normal_matrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec2 texture_coords;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;
	vertex_position = vec3(M * pos);
	vertex_normal = normalize(normal_matrix * normal);
	texture_coords = tex_coords;
}