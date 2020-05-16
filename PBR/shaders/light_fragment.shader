#version 430 core

uniform vec3 light_color;

out vec4 fragment_color;

void main() {
	fragment_color = vec4(light_color, 1.0f);
}