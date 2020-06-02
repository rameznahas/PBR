#version 430 core

in vec2 UV;

uniform sampler2D tex;

out vec4 color;

void main() {
	float gamma = 2.2f;
	color = texture(tex, UV);
	color.rgb = pow(color.rgb, vec3(1.0f / gamma));
}