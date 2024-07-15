#pragma once

#include "Shader.h"

class LightPreview
{
public:
    LightPreview();
    void Draw(const Shader& shader, const glm::vec3& lightColor);

private:
    void setup();

    unsigned int vao;
    unsigned int indexCount;
};
