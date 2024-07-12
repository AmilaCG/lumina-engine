#include "Mesh.h"

#include <glad/glad.h>

Mesh::Mesh(const std::vector<Vertex>& verticies,
           const std::vector<unsigned int>& indices,
           const std::vector<Texture>& textures,
           const bool isPbr)
    : verticies(verticies), indices(indices), textures(textures), isPbr(isPbr)
{
    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Bind vertex array object first and then bind and set vertex buffers
    glBindVertexArray(vao);

    // Bind to the vertex buffer and copy data into it
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(Vertex), &verticies[0], GL_STATIC_DRAW);

    // Bind to the element buffer and copy data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Vertex texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // Vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glBindVertexArray(0); // Unbind
}

unsigned int Mesh::Draw(Shader& shader)
{
    unsigned int albedoNr = 1;
    unsigned int metallicNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int aoNr = 1;

    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;

    shader.setBool("isPbr", isPbr);

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        // Activate proper texture unit before binding
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;
        std::string materialName;

        if (name == "texture_normal")
        {
            number = std::to_string(normalNr++);
        }

        if (isPbr)
        {
            if (name == "texture_albedo")
            {
                number = std::to_string(albedoNr++);
            }
            else if (name == "texture_metallic")
            {
                number = std::to_string(metallicNr++);
            }
            else if (name == "texture_roughness")
            {
                number = std::to_string(roughnessNr++);
            }
            else if (name == "texture_ao")
            {
                number = std::to_string(aoNr++);
            }

            materialName.append("materialPbr.").append(name).append(number);
        }
        else
        {
            if (name == "texture_diffuse")
            {
                number = std::to_string(diffuseNr++);
            }
            else if (name == "texture_specular")
            {
                number = std::to_string(specularNr++);
            }

            materialName.append("material.").append(name).append(number);
        }

        shader.setInt(materialName, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    // Reset active texture unit to 0 as a good practice. This is not mandatory.
    // https://community.khronos.org/t/glactivetexture-before-drawing/73757/2
    glActiveTexture(GL_TEXTURE0);

    // Draw
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    return indices.size();
}

void Mesh::deinit()
{
    // Doing cleanup in a separate function instead of the destructor because these meshes are stored
    // in a std::vector at Model class. Each time this vector grows, it deletes and recreate these
    // meshes so the OpenGL objects loses their references.
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}
