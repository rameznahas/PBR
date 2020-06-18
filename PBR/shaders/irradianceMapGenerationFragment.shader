#version 430 core

in vec3 uvw;

uniform samplerCube environmentMap;

const float PI = 3.14159265359f;
const float fullCircle = 2.0f * PI;
const float quarterCircle = 0.5f * PI;

out vec4 color;

void main() {
	vec3 N = normalize(uvw);
	vec3 right = cross(vec3(0.0f, 1.0f, 0.0f), N);
	vec3 up = cross(N, right);

	float sampleDelta = 0.025f;
	float nbSamples = 0.0f;
	vec3 irradiance = vec3(0.0f);
	for (float azimuth = 0.0f; azimuth < fullCircle; azimuth += sampleDelta) {
		for (float zenith = 0.0f; zenith < quarterCircle; zenith += sampleDelta) {
			// spherical coord. to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(zenith) * cos(azimuth), sin(zenith) * sin(azimuth), cos(zenith));
			// tangent to world space
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(environmentMap, sampleVec).rgb * cos(zenith) * sin(zenith);
			nbSamples++;
		}
	}
	irradiance *= PI;
	irradiance /= nbSamples;

	color = vec4(irradiance, 1.0f);
}