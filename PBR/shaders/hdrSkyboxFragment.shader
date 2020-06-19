#version 430 core

in vec3 uvw;

// size: 4 bytes
layout (std140, binding = 2) uniform toneMapping {	// base alignment	// aligned offset
	float exposure;									// 4 bytes			// 0 
};

uniform samplerCube skybox;

const vec3 gammaCorrection = vec3(1.0f / 2.2f);

out vec4 color;

void main() {
	vec3 hdrColor = texture(skybox, uvw).rgb;
	vec3 toneMapping = vec3(1.0f) - exp(-hdrColor * exposure);
	color = vec4(pow(toneMapping, gammaCorrection), 1.0f);
}