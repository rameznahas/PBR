#version 430 core

in VS_OUT {
	vec3 tPos;
	vec3 tLightPos;
	vec3 tViewPos;
	vec2 uv;
} fsIn;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform float heightScale;
uniform bool parallax;

const float minLayers = 8.0f;
const float maxLayers = 32.0f;

out vec4 color;

vec2 parallaxOffset(vec3 tViewDir);

void main() {
	vec3 tViewDir = normalize(fsIn.tViewPos - fsIn.tPos);
	vec2 uv = parallax ? parallaxOffset(tViewDir) : fsIn.uv;

	if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f) discard;

	vec3 col = texture(albedoMap, uv).rgb;
	vec3 tNorm = normalize(texture(normalMap, uv).xyz * 2.0f - 1.0f);

	vec3 ambient = 0.1f * col;

	vec3 tLightDir = normalize(fsIn.tLightPos - fsIn.tPos);
	float diffAngle = max(dot(tLightDir, tNorm), 0.0f);
	vec3 diffuse = diffAngle * col;

	vec3 H = normalize(tViewDir + tLightDir);
	float specAngle = max(dot(H, tNorm), 0.0f);
	vec3 specular = specAngle * vec3(0.2f);

	color = vec4(ambient + diffuse + specular, 1.0f);
}

vec2 parallaxOffset(vec3 tViewDir) {
	float nbLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), tViewDir), 0.0));
	float layerDepth = 1.0f / nbLayers;

	vec2 p = tViewDir.xy / tViewDir.z * heightScale;
	vec2 currentUV = fsIn.uv;
	float currentLayerDepth = 0.0f;
	float currentDepth = texture(heightMap, currentUV).r;
	vec2 deltaUV = p / nbLayers;

	while (currentLayerDepth < currentDepth) {
		currentUV -= deltaUV;
		currentDepth = texture(heightMap, currentUV).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevUV = currentUV + deltaUV;
	float afterDepth = currentDepth - currentLayerDepth;
	float beforeDepth = texture(heightMap, prevUV).r - currentLayerDepth + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);

	return mix(currentUV, prevUV, weight);
}