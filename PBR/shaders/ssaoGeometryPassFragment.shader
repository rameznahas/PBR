#version 430 core

struct Material {
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
};

in VS_OUT {
	vec3 vsPos;
	vec2 uv;
	mat3 vsTBN;
} fsIn;

uniform Material material1;

layout (location = 0) out vec3 vsPos;
layout (location = 1) out vec3 vsNorm;
layout (location = 2) out vec4 colorSpec;

void main() {
	vsPos = fsIn.vsPos;
	vsNorm = texture(material1.normalMap, fsIn.uv).rgb * 2.0f - 1.0f; // transform to [-1, 1] range
	vsNorm = normalize(fsIn.vsTBN * vsNorm);
	colorSpec.rgb = texture(material1.diffuseMap, fsIn.uv).rgb;
	colorSpec.a = texture(material1.specularMap, fsIn.uv).r;
}