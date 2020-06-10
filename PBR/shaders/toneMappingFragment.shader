#version 430 core

in VS_OUT {
	vec2 uv;
} fsIn;

uniform sampler2D hdrTex;
uniform sampler2D bloom;

out vec4 color;

void main() {
	vec3 hdrColor = texture(hdrTex, fsIn.uv).rgb + texture(bloom, fsIn.uv).rgb;
	vec3 toneMapping = hdrColor / (hdrColor + vec3(1.0f));

	color = vec4(toneMapping, 1.0f);
}