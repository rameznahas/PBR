#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glew.h"
#include "Shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uvs;
	glm::vec3 tangent;
};

struct Texture {
	GLuint id;
	std::string type;
};

class Mesh {
public:
	Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs);
	void draw(const Shader& shader, GLenum startingTexSlot, std::string suffix = "") const;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

