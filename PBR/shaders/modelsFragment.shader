#version 430 core

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 uv;
} fsIn;

uniform sampler2D tex;

out vec4 color;

void main() {
	color = texture(tex, fsIn.uv);
}