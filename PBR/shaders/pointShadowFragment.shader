#version 430 core

in vec3 fragPos;

uniform float farPlane;

void main() {
	gl_FragDepth = length(fragPos - vec3(0.0f)) / farPlane;
}