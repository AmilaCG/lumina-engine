#include "LightPreview.h"
#include <glad/glad.h>

LightPreview::LightPreview()
{
    setup();
}

void LightPreview::setup()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 3, (void*)0);

    glBindVertexArray(0);
}

void LightPreview::Draw(const Shader& shader, glm::vec3 lightColor)
{
    shader.setVec3("lightColor", lightColor);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
