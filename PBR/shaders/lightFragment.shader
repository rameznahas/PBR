#version 430 core

uniform vec3 lightColor;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bloomColor;

void main() {
	color = vec4(lightColor, 1.0f);
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	bloomColor = brightness > 2.5f ? color : vec4(0.0f, 0.0f, 0.0f, 1.0f);
}