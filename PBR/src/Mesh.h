#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glew.h"
#include "Shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uvs;
};

struct Texture {
	GLuint id;
	std::string type;
};

class Mesh {
public:
	Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs);
	void draw(Shader shader) const;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

