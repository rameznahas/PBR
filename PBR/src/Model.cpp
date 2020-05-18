#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Model.h"

Model::Model(std::string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		delete scene;
		return;
	}
	stbi_set_flip_vertically_on_load(true);

	dir = path.substr(0, path.find_last_of('/') + 1);

	process_node(scene->mRootNode, scene);
}

void Model::draw(Shader shader) const {
	for (unsigned int i = 0; i < meshes.size(); ++i)
		meshes[i].draw(shader);
}

void Model::process_node(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh mesh = process_mesh(ai_mesh, scene);
		meshes.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;

		vertex.position = glm::vec3(
			mesh->mVertices[i].x, 
			mesh->mVertices[i].y, 
			mesh->mVertices[i].z
		);

		vertex.normal = glm::vec3(
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z
		);

		if (mesh->mTextureCoords[0]) {
			vertex.uvs = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}
		else {
			vertex.uvs = glm::vec2(0.0f);
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
			indices.push_back(mesh->mFaces[i].mIndices[j]);

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	
	std::vector<Texture> diffuse = load_material_textures(material, aiTextureType_DIFFUSE, "diffuse");
	textures.insert(textures.end(), diffuse.begin(), diffuse.end());

	std::vector<Texture> specular = load_material_textures(material, aiTextureType_SPECULAR, "specular");
	textures.insert(textures.end(), specular.begin(), specular.end());

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::load_material_textures(aiMaterial* material, aiTextureType type, const char* type_name) {
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < material->GetTextureCount(type); ++i) {
		aiString path;
		material->GetTexture(type, i, &path);

		Texture texture;
		const char* file = path.C_Str();
		auto tex = loaded_textures.find(file);

		if (tex == loaded_textures.end()) {
			texture.id = load_texture(file);
			texture.type = type_name;
			loaded_textures[file] = texture;
		}
		else {
			texture = tex->second;
		}

		textures.push_back(texture);
	}
	
	return textures;
}

GLuint Model::load_texture(const char* path) {
	std::string file_path = dir + path;

	GLuint texture;
	glGenTextures(1, &texture);

	int width, height, channels;
	unsigned char* data = stbi_load(file_path.c_str(), &width, &height, &channels, 0);
	if (data) {
		GLenum format;
		if (channels == 1) format = GL_RED;
		else if (channels == 2) format = GL_RG;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "ERROR::TEXTURE::LOADING" << std::endl;
	}

	stbi_image_free(data);

	return texture;
}