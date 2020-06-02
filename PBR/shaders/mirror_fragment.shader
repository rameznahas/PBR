#version 430 core

// sizeof(Light): 15 * sizeof(float) + 4
// sizeof(Light): 64 bytes
struct Light {			
	vec3 color;			
	float kc;			
	vec3 ambient;		
	float kl;			
	vec3 diffuse;		
	float kq;			
	vec3 specular;		// empty 4 bytes padding	
};						

// sizeof(Point_Light): sizeof(Light) + 3 * sizeof(float) + 4
// sizeof(Point_Light): 80 bytes
struct Point_Light {	
	Light light;		
	vec3 position;		// empty 4 bytes padding
};						

// sizeof(Directional_Light): sizeof(Light) + 3 * sizeof(float) + 4
// sizeof(Directional_Light): 80 bytes
struct Directional_Light {
	Light light;		
	vec3 direction;		// empty 4 bytes padding
};						

struct Spotlight {
	Light light;
	vec3 position;
	float inner_cutoff;
	vec3 direction;
	float outer_cutoff;
};

#define NB_POINT_LIGHTS 4

in vec3 vertex_position;
in vec3 vertex_normal;

layout (std140, binding = 0) uniform lighting {			// base alignment					// aligned offset
	Directional_Light dir_light;						// 80 bytes							// 0
	Point_Light point_lights[NB_POINT_LIGHTS];			// 320 (4 * 80) bytes				// 80
	vec3 cam_position;									// 16 (12 + 4 pad) bytes			// 400
};																							// sizeof(lighting): 416 bytes
													

uniform samplerCube skybox;

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
	vec3 halfway_dir = normalize(light_dir + cam_dir);
	float spec_angle = pow(max(dot(halfway_dir, vertex_normal), 0.0f), 4096.0f);
	vec3 specular = light.color * light.specular * spec_angle;

	return specular;
}

float attenuation(Light light, vec3 light_position) {
	float distance = length(light_position - vertex_position);
	return 1.0f / (light.kc + light.kl * distance + light.kq * pow(distance, 2));
}