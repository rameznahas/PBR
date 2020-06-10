#version 430 core
#define NB_WEIGHTS 5

in VS_OUT {
	vec2 uv;
} fsIn;

uniform sampler2D bloom;
uniform bool horizontal;

float weights[NB_WEIGHTS] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

out vec4 color;

void main() {
	vec2 texOffset = 1.0f / textureSize(bloom, 0);
	vec3 col = texture(bloom, fsIn.uv).rgb * weights[0];

	if (horizontal) {
		for (unsigned int i = 1; i < NB_WEIGHTS; ++i) {
			col += texture(bloom, fsIn.uv + vec2(texOffset.x * i, 0.0f)).rgb * weights[i];
			col += texture(bloom, fsIn.uv - vec2(texOffset.x * i, 0.0f)).rgb * weights[i];
		}
	}
	else {
		for (unsigned int i = 1; i < NB_WEIGHTS; ++i) {
			col += texture(bloom, fsIn.uv + vec2(0.0f, texOffset.y * i)).rgb * weights[i];
			col += texture(bloom, fsIn.uv - vec2(0.0f, texOffset.y * i)).rgb * weights[i];
		}
	}
	color = vec4(col, 1.0f);
}