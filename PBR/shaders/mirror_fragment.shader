#version 430 core

struct Light {
	vec3 color;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float kc;
	float kl;
	float kq;
};

struct Point_Light {
	Light light;
	vec3 position;
};

struct Directional_Light {
	Light light;
	vec3 direction;
};

struct Spotlight {
	Light light;
	vec3 position;
	vec3 direction;
	float inner_cutoff;
	float outer_cutoff;
};

#define NB_POINT_LIGHTS 4

in vec3 vertex_position;
in vec3 vertex_normal;

uniform vec3 cam_position;
uniform samplerCube skybox;
uniform Directional_Light dir_light;
uniform Point_Light point_lights[NB_POINT_LIGHTS];

out vec4 fragment_color;

vec3 compute_light(Light light, vec3 light_direction);
float attenuation(Light light, vec3 light_position);

void main() {
	vec3 color = compute_light(dir_light.light, -dir_light.direction);

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Point_Light point_light = point_lights[i];
		color += compute_light(point_light.light, point_light.position - vertex_position)
			* attenuation(point_light.light, point_light.position);
	}

	vec3 cam_dir = normalize(vertex_position - cam_position);
	vec3 refl_dir = reflect(cam_dir, vertex_normal);

	fragment_color = vec4(color, 1.0f) + texture(skybox, refl_dir);
}

vec3 compute_light(Light light, vec3 light_direction) {
	vec3 light_dir = normalize(light_direction);

	vec3 cam_dir = normalize(cam_position - vertex_position);
	vec3 reflected_dir = reflect(-light_dir, vertex_normal);
	float spec_angle = pow(max(dot(cam_dir, reflected_dir), 0.0f), 1024.0f);
	vec3 specular = light.color * light.specular * spec_angle;

	return specular;
}

float attenuation(Light light, vec3 light_position) {
	float distance = length(light_position - vertex_position);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2));
}