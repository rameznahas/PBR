#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 uv;

uniform mat4 M;
uniform mat4 MVP;
uniform mat3 NM;
uniform vec3 wLightPos;
uniform vec3 wViewPos;

out VS_OUT {
	vec3 tPos;
	vec3 tLightPos;
	vec3 tViewPos;
	vec2 uv;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;

	vec3 T = normalize(NM * tangent);
	vec3 N = normalize(NM * normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	mat3 invTBN = transpose(mat3(T, B, N));

	vsOut.tPos = invTBN * vec3(M * pos);
	vsOut.tLightPos = invTBN * wLightPos;
	vsOut.tViewPos = invTBN * wViewPos;
	vsOut.uv = uv;
}