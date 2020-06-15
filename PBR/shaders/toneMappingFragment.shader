#version 430 core

in VS_OUT {
	vec2 uv;
} fsIn;

uniform sampler2D hdrTex;
uniform sampler2D bloom;
uniform float exposure;

const vec3 gammaCorrection = vec3(1.0f / 2.2f);

out vec4 color;

void main() {
	vec3 hdrColor = texture(hdrTex, fsIn.uv).rgb + texture(bloom, fsIn.uv).rgb;
	vec3 toneMapping = vec3(1.0f) - exp(-hdrColor * exposure);

	color = vec4(pow(toneMapping, gammaCorrection), 1.0f);
}