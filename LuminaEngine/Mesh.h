#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string name;
};

class Mesh
{
public:
    Mesh(const std::vector<Vertex>& verticies,
         const std::vector<unsigned int>& indices,
         const std::vector<Texture>& textures,
         bool isPbr);
    unsigned int Draw(Shader& shader);
    void deinit();

public:
    // Mesh data
    std::vector<Vertex>         verticies;
    std::vector<unsigned int>   indices;
    std::vector<Texture>        textures;

private:
    void setupMesh();

private:
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    bool isPbr;
};
