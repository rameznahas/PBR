#version 430 core

in vec3 uvw;

uniform samplerCube skybox;

const vec3 gammaCorrection = vec3(1.0f / 2.2f);

out vec4 color;

void main() {
	vec3 hdrColor = texture(skybox, uvw).rgb;
	vec3 toneMapping = hdrColor / (hdrColor + vec3(1.0f));
	color = vec4(pow(toneMapping, gammaCorrection), 1.0f);
}