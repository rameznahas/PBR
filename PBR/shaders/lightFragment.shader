#version 430 core

uniform vec3 lightColor;

layout (location = 0) out vec3 color;
layout (location = 1) out vec3 bloomColor;

void main() {
	color = lightColor;
	float brightness = dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
	bloomColor = brightness > 5.0f ? color : vec3(0.0f);
}