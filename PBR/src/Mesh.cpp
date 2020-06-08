#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs)
	:
	vertices(verts),
	indices(inds),
	textures(texs)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void Mesh::draw(const Shader& shader, GLenum startingTexSlot, std::string suffix) const {
	unsigned int nb_diffuse = 1;
	unsigned int nb_specular = 1;
	unsigned int nb_normals = 1;
	unsigned int slot = (unsigned int)(startingTexSlot % GL_TEXTURE0);

	for (unsigned int i = 0; i < textures.size(); ++i) {
		std::string nb;
		std::string type = textures[i].type;

		if (type == "diffuse") nb = std::to_string(nb_diffuse++);
		else if (type == "specular") nb = std::to_string(nb_specular++);
		else if (type == "normal") nb = std::to_string(nb_normals++);

		glActiveTexture(startingTexSlot + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
		std::string test = "material" + nb + "." + type + suffix;
		shader.set_int(("material" + nb + "." + type + suffix).c_str(), slot + i);
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}