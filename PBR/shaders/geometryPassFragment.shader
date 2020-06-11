#version 430 core

struct Material {
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
};

in VS_OUT {
	vec3 worldPos;
	vec2 uv;
	mat3 TBN;
} fsIn;

uniform Material material1;

layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 worldNorm;
layout (location = 2) out vec4 colorSpec;

void main() {
	worldPos = fsIn.worldPos;
	worldNorm = texture(material1.normalMap, fsIn.uv).rgb * 2.0f - 1.0f; // transform to [-1, 1] range
	worldNorm = normalize(fsIn.TBN * worldNorm);
	colorSpec.rgb = texture(material1.diffuseMap, fsIn.uv).rgb;
	colorSpec.a = texture(material1.specularMap, fsIn.uv).r;
}