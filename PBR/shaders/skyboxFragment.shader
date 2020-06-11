#version 430 core

in vec3 uvw;

uniform samplerCube skybox;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bloomColor;

void main() {
	color = texture(skybox, uvw);
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	bloomColor = brightness > 2.5f ? color : vec4(0.0f, 0.0f, 0.0f, 1.0f);
}