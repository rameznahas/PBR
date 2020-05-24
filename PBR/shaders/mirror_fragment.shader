#version 430 core

uniform vec3 cam_position;
uniform samplerCube skybox;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec4 fragment_color;

void main() {
	vec3 cam_dir = normalize(vertex_position - cam_position);
	vec3 refl_dir = reflect(cam_dir, vertex_normal);

	fragment_color = texture(skybox, refl_dir);
}