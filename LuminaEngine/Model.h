#pragma once

#include <string>
#include <vector>
#include <assimp/scene.h>
#include "Shader.h"
#include "Mesh.h"

class Model
{
public:
    explicit Model(const std::string& path);
    unsigned int Draw(Shader& shader);

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    unsigned int loadTexture(const std::string& fileName, const std::string& directory, bool gammaCorrection);

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> texturesLoaded;
};
