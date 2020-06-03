#version 430 core

struct Light {
	vec3 color;
	float kc;
	vec3 ambient;
	float kl;
	vec3 diffuse;
	float kq;
	vec3 specular;
};

struct DirectionalLight {
	Light light;
	vec3 direction;
};

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 uv;
} fsIn;

// size: 96 bytes
layout (std140, binding = 1) uniform lighting {		// base alignment	// aligned offset
	DirectionalLight directionalLight;				// 80 bytes			// 0
	vec3 viewPos;									// 16 bytes			// 80
};

uniform sampler2D tex;

out vec4 color;

void main() {
	vec3 col = texture(tex, fsIn.uv).rgb;

	Light light = directionalLight.light;

	vec3 ambient = light.ambient * col;

	vec3 lightDir = normalize(-directionalLight.direction);
	float diffAngle = max(dot(lightDir, fsIn.normal), 0.0f);
	vec3 diffuse = light.color * light.diffuse * diffAngle * col;

	vec3 viewDir = normalize(viewPos - fsIn.position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specAngle = pow(max(dot(halfwayDir, fsIn.normal), 0.0f), 64.0f);
	vec3 specular = light.color * light.specular * specAngle * vec3(1.0f);

	color = vec4(ambient + diffuse + specular, 1.0f);
}