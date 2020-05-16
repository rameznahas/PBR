#pragma once
#include "glm/glm.hpp"

struct Material {
	Material() = default;
	Material(glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
		:
		ambient(ambi),
		diffuse(diff),
		specular(spec),
		shininess(shin)
	{}

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
};
