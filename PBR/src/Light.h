#pragma once
#include "glm/glm.hpp"

struct Light {
	Light(glm::vec3 col, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic)
		:
		color(col),
		ambient(ambi),
		diffuse(diff),
		specular(spec),
		kc(constant),
		kl(linear),
		kq(quadratic)
	{}

	glm::vec3 color;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float kc;
	float kl;
	float kq;
};

struct Point_Light : public Light {
	Point_Light(glm::vec3 pos, glm::vec3 col, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float constant = 0.0f, float linear = 0.0f, float quadratic = 0.0f)
		:
		Light(col, ambi, diff, spec, constant, linear, quadratic),
		position(pos)
	{}

	glm::vec3 position;
};

struct Directional_Light : public Light {
	Directional_Light(glm::vec3 dir, glm::vec3 col, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float constant = 0.0f, float linear = 0.0f, float quadratic = 0.0f)
		:
		Light(col, ambi, diff, spec, constant, linear, quadratic),
		direction(glm::normalize(dir))
	{}

	glm::vec3 direction;
};

struct Spotlight : public Light {
	Spotlight(glm::vec3 pos, glm::vec3 dir, glm::vec3 col, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic, float inner_cut, float outer_cut)
		:
		Light(col, ambi, diff, spec, constant, linear, quadratic),
		position(pos),
		direction(dir),
		inner_cutoff(glm::cos(glm::radians(inner_cut))),
		outer_cutoff(glm::cos(glm::radians(outer_cut)))
	{}

	glm::vec3 position;
	glm::vec3 direction;
	float inner_cutoff;
	float outer_cutoff;
};