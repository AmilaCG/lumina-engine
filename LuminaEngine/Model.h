#pragma once

#include <string>
#include <vector>
#include <assimp/scene.h>
#include "Shader.h"
#include "Mesh.h"

class Model
{
public:
    Model(const std::string& path, bool isPbr);
    ~Model();
    unsigned int Draw(Shader& shader);

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

private:
    // TODO: Save meshes in a fixed size array so we can use ~Meshes() to delete OpenGL objects
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> texturesLoaded;
    bool isPbr;
};
