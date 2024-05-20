#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    /// <summary>
    /// Use/Activate the shader program
    /// </summary>
    void use();

    unsigned int getProgramID();

    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat3(const std::string& name, glm::mat3 value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setVec4(const std::string& name, glm::vec4 value) const;

private:
    void compileAndLink(const char* vShaderCode, const char* fShaderCode);
    void checkCompileErrors(unsigned int shader, std::string type);

private:
    unsigned int programID;
};
