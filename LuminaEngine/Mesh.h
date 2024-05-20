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
    Mesh(std::vector<Vertex> verticies, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);

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
};
