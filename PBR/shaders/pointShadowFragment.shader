#version 430 core

in vec3 fragPos;

uniform float farPlane;
uniform vec3 lightPos;

void main() {
	gl_FragDepth = length(fragPos - lightPos) / farPlane;
}