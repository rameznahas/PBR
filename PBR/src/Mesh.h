#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glew.h"
#include "Shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

struct Texture {
	GLuint id;
	std::string type;
};

class Mesh {
public:
	Mesh(std::vector<Vertex> verts, std::vector<GLuint> inds, std::vector<Texture> texs);
	void draw(Shader shader);

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

