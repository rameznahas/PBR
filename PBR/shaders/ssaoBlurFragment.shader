#version 430 core

in VS_OUT {
	vec2 uv;
} fsIn;

uniform sampler2D ssao;

out float color;

void main() {
	vec2 texelSize = 1.0f / textureSize(ssao, 0);
	float result = 0.0f;
	for (int x = -2; x < 2; ++x) {
		for (int y = -2; y < 2; ++y) {
			vec2 offset = texelSize * vec2(float(x), float(y));
			result += texture(ssao, fsIn.uv + offset).r;
		}
	}
	color = result / 16.0f;
}