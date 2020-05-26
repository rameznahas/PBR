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
	vec3 position;
	Light light;
};

struct Directional_Light {
	vec3 direction;
	Light light;
};

struct Spotlight {
	vec3 position;
	vec3 direction;
	Light light;
	float inner_cutoff;
	float outer_cutoff;
};

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	sampler2D emission;
	float shininess;
};

#define NB_POINT_LIGHTS 4

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 UV;

uniform Directional_Light dir_light;
uniform Point_Light point_lights[NB_POINT_LIGHTS];
uniform Spotlight spotlight;
uniform Material material;
uniform vec3 cam_position;

out vec4 fragment_color;

vec3 compute_light(Light light, vec3 light_direction);
float attenuation(Light light, vec3 light_position);
float intensity(vec3 frag_light_dir, vec3 light_direction, float inner_cutoff, float outer_cutoff);

void main() {
	vec3 color = compute_light(dir_light.light, -dir_light.direction);

	for (unsigned int i = 0; i < NB_POINT_LIGHTS; ++i) {
		Point_Light point_light = point_lights[i];
		color += compute_light(point_light.light, point_light.position - vertex_position) 
			* attenuation(point_light.light, point_light.position);
	}

	/*vec3 frag_light_dir = spotlight.position - vertex_position;
	color += compute_light(spotlight.light, frag_light_dir) 
		* attenuation(spotlight.light, spotlight.position) 
		* intensity(frag_light_dir, spotlight.direction, spotlight.inner_cutoff, spotlight.outer_cutoff);*/

	fragment_color = vec4(color, 1.0f);
}

vec3 compute_light(Light light, vec3 light_direction) {
	vec3 object_diffuse = texture(material.diffuse1, UV).rgb;

	vec3 ambient = light.ambient * object_diffuse;

	vec3 light_dir = normalize(light_direction);
	float diff_angle = max(dot(light_dir, vertex_normal), 0.0f);
	vec3 diffuse = light.color * light.diffuse * diff_angle * object_diffuse;

	vec3 cam_dir = normalize(cam_position - vertex_position);
	vec3 reflected_dir = reflect(-light_dir, vertex_normal);
	float spec_angle = pow(max(dot(cam_dir, reflected_dir), 0.0f), 32.0f);
	vec3 specular = light.color * light.specular * spec_angle * texture(material.specular1, UV).r;

	return ambient + diffuse + specular;
}

float attenuation(Light light, vec3 light_position) {
	float distance = length(light_position - vertex_position);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2));
}

float intensity(vec3 frag_light_dir, vec3 light_direction, float inner_cutoff, float outer_cutoff) {
	float theta = dot(normalize(frag_light_dir), normalize(-light_direction));
	float epsilon = inner_cutoff - outer_cutoff;
	return clamp((theta - outer_cutoff) / epsilon, 0.0f, 1.0f);
}