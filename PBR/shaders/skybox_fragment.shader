#version 430 core

in vec3 uvw;

uniform samplerCube skybox;

out vec4 fragment_color;

void main() {
	fragment_color = texture(skybox, uvw);
}