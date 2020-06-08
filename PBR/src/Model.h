#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Shader.h"
#include "Mesh.h"

class Model {
public:
	Model(std::string path, bool optimizeMesh);
	void draw(const Shader& shader, GLenum startingTexSlot, std::string suffix = "") const;

	glm::mat4 M;

private:
	void process_node(aiNode* node, const aiScene* scene);
	Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> load_material_textures(aiMaterial* material, aiTextureType type, const char* type_name);
	GLuint load_texture(const char* path);

	std::vector<Mesh> meshes;
	std::unordered_map<std::string, Texture> loaded_textures;
	std::string dir;
};

