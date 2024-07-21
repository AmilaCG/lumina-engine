#include "Model.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "TextureUtils.h"

Model::Model(const std::string& path, const bool isPbr) : isPbr(isPbr)
{
    loadModel(path);
}

Model::~Model()
{
    for (auto& [id, type, name] : texturesLoaded)
    {
        glDeleteTextures(1, &id);
    }

    for (auto& mesh : meshes)
    {
        mesh.deinit();
    }
}

unsigned int Model::Draw(Shader& shader)
{
    unsigned int indiceCount = 0;
    for (Mesh& mesh : meshes)
    {
        indiceCount += mesh.Draw(shader);
    }
    return indiceCount;
}

void Model::loadModel(const std::string& path)
{
    Assimp::Importer importer;
    constexpr unsigned int FLAGS = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs;
    const aiScene* scene = importer.ReadFile(path, FLAGS);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    this->directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> verticies;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex = {};

        // Process vertex position
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        // Process vertex normal
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }

        // Process vertex tangent
        if (mesh->HasTangentsAndBitangents())
        {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
        }

        // Process texture coordinates
        // Assimp allows a model to have up to 8 different texture coordinates per vertex.
        // We are only going to use the first set and ignore the rest.
        if (mesh->mTextureCoords[0]) // Does the mesh contain texture coords?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        verticies.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        if (isPbr)
        {
            std::vector<Texture> albedoMaps =
                loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_albedo");
            textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

            std::vector<Texture> metallicMaps =
                loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_metallic");
            textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

            std::vector<Texture> roughnessMaps =
                loadMaterialTextures(material, aiTextureType_SHININESS, "texture_roughness");
            textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

            std::vector<Texture> aoMaps =
                loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ao");
            textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
        }
        else
        {
            std::vector<Texture> diffuseMaps =
                loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            std::vector<Texture> specularMaps =
                loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        std::vector<Texture> normalMaps =
                    loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    meshes.emplace_back(verticies, indices, textures, isPbr);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < this->texturesLoaded.size(); j++)
        {
            if (std::strcmp(this->texturesLoaded[j].name.data(), str.C_Str()) == 0)
            {
                textures.push_back(texturesLoaded[j]);
                skip = true;
                break;
            }
        }
        if (skip) { continue; }

        // Diffuse and AO textures are almost always in the sRGB space. Therefore, only convert
        // these textures into linear space when loading.
        bool shouldCorrectGamma =
            type == aiTextureType_DIFFUSE ||
            type == aiTextureType_AMBIENT;
        Texture texture;
        std::string path = this->directory + "/" + str.C_Str();
        texture.id = TextureUtils::loadTexture(path, shouldCorrectGamma);
        texture.type = typeName;
        texture.name = str.C_Str();
        textures.push_back(texture);
        texturesLoaded.push_back(texture);
    }
    return textures;
}
