#version 430 core

in vec3 uvw;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591f, 0.3183f);

out vec4 color;

void main() {
	vec3 dir = normalize(uvw);
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y)) * invAtan + 0.5f;

	color = texture(equirectangularMap, uv);
}