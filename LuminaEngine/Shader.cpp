#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // Ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (const std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ " << e.what() << std::endl;
    }

    compileAndLink(vertexCode.c_str(), fragmentCode.c_str());
}

Shader::~Shader()
{
    glDeleteProgram(programID);
}

void Shader::use()
{
    glUseProgram(programID);
}

unsigned int Shader::getProgramID()
{
    return programID;
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setMat3(const std::string& name, glm::mat3 value) const
{
    int uniformLoc = glGetUniformLocation(programID, name.c_str());
    glUniformMatrix3fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
    int uniformLoc = glGetUniformLocation(programID, name.c_str());
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, glm::vec3 value) const
{
    int uniformLoc = glGetUniformLocation(programID, name.c_str());
    glUniform3fv(uniformLoc, 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, glm::vec4 value) const
{
    int uniformLoc = glGetUniformLocation(programID, name.c_str());
    glUniform4fv(uniformLoc, 1, glm::value_ptr(value));
}

void Shader::compileAndLink(const char* vShaderCode, const char* fShaderCode)
{
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    programID = glCreateProgram();
    glAttachShader(programID, vertex);
    glAttachShader(programID, fragment);
    glLinkProgram(programID);
    checkCompileErrors(programID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << 
                "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << 
                "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
