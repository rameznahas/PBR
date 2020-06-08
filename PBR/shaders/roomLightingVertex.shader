#version 430 core
#define NB_POINT_LIGHTS 6

struct Light {
	vec3 color;
	float kc;
	vec3 ambient;
	float kl;
	vec3 diffuse;
	float kq;
	vec3 specular;
};

struct PointLight {
	Light light;
	vec3 position;
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

// size: 176 bytes
layout (std140, binding = 0) uniform transforms {	// base alignment	// aligned offset
	mat4 M;											// 64 bytes			// 0
	mat4 MVP;										// 64 bytes			// 64
	mat3 normalMatrix;								// 48 bytes			// 128
};

// size: 496 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	PointLight pointLight[NB_POINT_LIGHTS];			// 480 bytes		// 0
	vec3 worldViewPos;									//  12 bytes		// 480
	float farPlane;									//   4 bytes		// 492
};

out VS_OUT {
	vec3 worldPos;
	vec3 tangentPos;
	vec3 tangentViewPos;
	vec3 tangentLightPos[NB_POINT_LIGHTS];
	vec2 uv;
} vsOut;

void main() {
	vec4 pos = vec4(position, 1.0f);
	gl_Position = MVP * pos;

	vec3 T = normalize(normalMatrix * tangent);
	vec3 B = normalize(normalMatrix * biTangent);
	vec3 N = normalize(normalMatrix * normal);
	T = normalize(T - dot(T, N) * N);

	// we can transpose instead of inverse because TBN is an orthogonal matrix, with
	// the property that a transpose operation is similar to an inverse operation.
	// transpose is less costly than inverse
	mat3 inverseTBN = transpose(mat3(T, B, N));

	vsOut.worldPos = vec3(M * pos);
	vsOut.tangentPos = inverseTBN * vsOut.worldPos;
	vsOut.tangentViewPos = inverseTBN * worldViewPos;
	vsOut.uv = uv;
	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		vsOut.tangentLightPos[i] = inverseTBN * pointLight[i].position;
	}
}