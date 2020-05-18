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
	Model(std::string path);
	void draw(Shader shader) const;

private:
	void process_node(aiNode* node, const aiScene* scene);
	Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> load_material_textures(aiMaterial* material, aiTextureType type, const char* type_name);
	GLuint load_texture(const char* path);

	std::vector<Mesh> meshes;
	std::unordered_map<std::string, Texture> loaded_textures;
	std::string dir;
};

